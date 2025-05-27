//
// Created by jack on 10/13/2024.
//

#ifndef DRAW2DBILLBOARD_H
#define DRAW2DBILLBOARD_H
#include "Shader.h"
#include "BillboardInstanceShader.h"
#include "Scene.h"

inline void draw2DBillboard(ImVec2 size, float xCropping = 0.0f)
{

    ImVec2 windowStart = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    static jl::Shader shader(
        R"glsl(
            #version 330 core

            layout(location = 0) in vec2 aPos;     // 2D position (0.0 to 1.0)
            layout(location = 1) in vec2 aTexCoord; // Texture coordinates

            out vec2 tcoord;

            uniform vec2 uPos;  // Position of the quad in NDC
            uniform vec2 uSize; // Size of the quad in NDC
            uniform float time;
            uniform float characterNum;
            uniform float animationIndex;
            uniform float startTime;
            uniform float timeScale;
            uniform vec3 instanceDirection;
            uniform vec3 camPos;
            uniform vec3 instancePosition;

            void main()
            {
                // Scale and translate the vertex position to NDC
                vec2 scaledPos = aPos * uSize + uPos;
                gl_Position = vec4(scaledPos, 0.0, 1.0);

                // Calculate animation frame
                float animationFrame = floor(mod((time - startTime) * (30.0 * timeScale), 16.0f));

                // Calculate direction-based offset for sprite sheet
                vec3 dirToCam = normalize(camPos - instancePosition);
                float dotProd = dot(dirToCam, instanceDirection * -1.0);
                vec3 crossProd = cross(instanceDirection * -1.0, dirToCam);
                float angle = atan(crossProd.y, dotProd);
                float normAngle = (angle + 3.1415926535897932384626433832795) / (2.0 * 3.1415926535897932384626433832795);
                float offset = min(round(normAngle * 16.0), 15.0);

                // Calculate base UV coordinates
                vec2 baseUV = vec2((characterNum * 0.25) + (animationFrame * (1.0 / 64.0)), offset / 16.0);

                // Apply texture coordinates based on vertex position
                tcoord = vec2(baseUV.x + aTexCoord.x * (1.0 / 64.0), baseUV.y + aTexCoord.y * (1.0 / 16.0));
            }
        )glsl",
        R"glsl(
            #version 330 core
            in vec2 tcoord;
            out vec4 FragColor;
            uniform sampler2DArray ourTexture;
            uniform float animationIndex;
            uniform float hidden;

            uniform vec2 ndcMin;
            uniform vec2 ndcMax;

            void main()
            {
                vec4 texColor = texture(ourTexture, vec3(tcoord, animationIndex));
                FragColor = texColor;
                if(texColor.a < 0.1 || hidden > 0.2f) {
                    discard;
                }
                if (gl_FragCoord.x < ndcMin.x || gl_FragCoord.x > ndcMax.x ||
                    gl_FragCoord.y > ndcMax.y || gl_FragCoord.y < ndcMin.y) {
                    discard;
                }
            }
        )glsl",
        "2dBillboardShader"
    );

    static GLuint vao = 0;
    static GLuint vbo = 0;

    if (vao == 0)
    {
        // Simple quad vertices (position + texture coordinates)
        float vertices[] = {
            // Position  // TexCoord
            0.0f, 0.0f,  0.0f, 1.0f,  // Bottom-left
            1.0f, 0.0f,  1.0f, 1.0f,  // Bottom-right
            1.0f, 1.0f,  1.0f, 0.0f,  // Top-right

            1.0f, 1.0f,  1.0f, 0.0f,  // Top-right
            0.0f, 1.0f,  0.0f, 0.0f,  // Top-left
            0.0f, 0.0f,  0.0f, 1.0f   // Bottom-left
        };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coordinate attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    // Get player data from ECS
    auto& renderComponent = theScene.our<RenderComponent>();
    auto& billboard = renderComponent.billboard;
    auto& animation_state = renderComponent.animation_state;
    auto& camera = theScene.our<jl::Camera>();

    // Get current ImGui cursor position
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    // Calculate cropped size (how much ImGui space we take up)
    ImVec2 croppedSize = ImVec2(size.x - (xCropping * 2.0f), size.y);

    // Calculate the actual render position (centered in the cropped space)
    ImVec2 renderPos = ImVec2(cursorPos.x - xCropping, cursorPos.y);

    // Get screen dimensions
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;

    // Convert render position and full size to NDC for OpenGL
    ImVec2 ndcPos;
    ndcPos.x = (renderPos.x / screenSize.x) * 2.0f - 1.0f;
    ndcPos.y = 1.0f - (renderPos.y / screenSize.y) * 2.0f;

    ImVec2 ndcSize;
    ndcSize.x = (size.x / screenSize.x) * 2.0f;  // Use full size for rendering
    ndcSize.y = (size.y / screenSize.y) * 2.0f;  // Use full size for rendering

    // Adjust for OpenGL coordinate system
    ndcPos.y -= ndcSize.y;

    // Disable depth testing for 2D rendering
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    // Use the shader
    glUseProgram(shader.shaderID);
    glBindVertexArray(vao);

    // Set uniforms
    static GLint posUniform = glGetUniformLocation(shader.shaderID, "uPos");
    static GLint sizeUniform = glGetUniformLocation(shader.shaderID, "uSize");
    static GLint timeUniform = glGetUniformLocation(shader.shaderID, "time");
    static GLint characterNumUniform = glGetUniformLocation(shader.shaderID, "characterNum");
    static GLint animationIndexUniform = glGetUniformLocation(shader.shaderID, "animationIndex");
    static GLint startTimeUniform = glGetUniformLocation(shader.shaderID, "startTime");
    static GLint timeScaleUniform = glGetUniformLocation(shader.shaderID, "timeScale");
    static GLint instanceDirectionUniform = glGetUniformLocation(shader.shaderID, "instanceDirection");
    static GLint camPosUniform = glGetUniformLocation(shader.shaderID, "camPos");
    static GLint instancePositionUniform = glGetUniformLocation(shader.shaderID, "instancePosition");
    static GLint hiddenUniform = glGetUniformLocation(shader.shaderID, "hidden");
    static GLint textureUniform = glGetUniformLocation(shader.shaderID, "ourTexture");

    static GLint ndcm = glGetUniformLocation(shader.shaderID, "ndcMin");
    static GLint ndcma = glGetUniformLocation(shader.shaderID, "ndcMax");

    glUniform2f(posUniform, ndcPos.x, ndcPos.y);
    glUniform2f(sizeUniform, ndcSize.x, ndcSize.y);
    glUniform1f(timeUniform, static_cast<float>(glfwGetTime()));
    glUniform1f(characterNumUniform, billboard.characterNum);
    glUniform1f(animationIndexUniform, animation_state.actionNum);
    glUniform1f(startTimeUniform, animation_state.timestarted);
    glUniform1f(timeScaleUniform, animation_state.timescale);
    auto normd = glm::vec3(0.f, 0.f, 1.f);
    normd = glm::normalize(normd);
    glUniform3f(instanceDirectionUniform, normd.x, normd.y, normd.z);

    auto fakecampos = glm::vec3(billboard.position.x, billboard.position.y, billboard.position.z);
    double xp = 0., yp = 0.;
    glfwGetCursorPos(theScene.window, &xp, &yp);
    fakecampos.x +=  (renderPos.x + xCropping + xCropping + (croppedSize.x/2)) - xp;
    fakecampos.y += (renderPos.y + (croppedSize.y / 2)) - yp;
    fakecampos.z += 100.0f;

    glUniform3f(camPosUniform, fakecampos.x, fakecampos.y, fakecampos.z);


    glUniform3f(instancePositionUniform, billboard.position.x, billboard.position.y, billboard.position.z);
    glUniform1f(hiddenUniform, billboard.hidden);

    glUniform1i(textureUniform, 2);


    // Convert your windowStart and windowSize to pixel coordinates for clipping
    ImVec2 clipMin = windowStart;
    ImVec2 clipMax = ImVec2(windowStart.x + windowSize.x, windowStart.y + windowSize.y);

    // Then pass these to the shader
    glUniform2f(ndcma, clipMax.x, clipMax.y);
    glUniform2f(ndcm, clipMin.x, clipMin.y);


    // Draw the quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Restore depth test state
    if (depthTestEnabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    // Advance ImGui cursor by the cropped size only
    ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + croppedSize.y));
}
#endif //DRAW2DBILLBOARD_H
