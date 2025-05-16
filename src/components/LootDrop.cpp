//
// Created by jack on 5/15/2025.
//

#include "LootDrop.h"

#include "NPPositionComponent.h"
#include "../ModelLoader.h"
#include "../Shader.h"
#include "../Scene.h"
void renderLootDrops(entt::registry& reg, Scene* scene)
{

    static jl::Shader shader(
        R"glsl(
            #version 330 core
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec2 inTexCoord;

            // Function to create a rotation matrix for yaw (around Y-axis)
            mat4 getYawRotationMatrix(float yaw) {
                float rad = radians(yaw); // Convert degrees to radians
                return mat4(
                    cos(rad), 0.0, sin(rad), 0.0,
                    0.0,      1.0, 0.0,      0.0,
                    -sin(rad), 0.0, cos(rad), 0.0,
                    0.0,      0.0, 0.0,      1.0
                );
            }

            uniform float rot; // Yaw rotation in degrees (0-360)

            out vec2 TexCoord;

            uniform mat4 mvp; // Model-View-Projection matrix
            uniform vec3 pos;  // Translation in world space
            out vec3 vertPos;
            void main()
            {
                // Apply yaw rotation to the local coordinates
                mat4 rotmat = getYawRotationMatrix(rot);
                vec4 rotatedPosition = rotmat * vec4(inPosition, 1.0);

                // Translate the rotated position by `pos`
                vec4 worldPosition = vec4(rotatedPosition.xyz + pos, 1.0);

                // Transform to clip space using the MVP matrix
                gl_Position = mvp * worldPosition;

                // Pass texture coordinates to the fragment shader
                TexCoord = inTexCoord;
                vertPos = worldPosition.xyz;
            }
        )glsl",
        R"glsl(
            #version 330 core
            out vec4 FragColor;

            uniform float hideClose;

            in vec2 TexCoord;
            in vec3 vertPos;

            uniform sampler2D texture1;
            uniform vec3 camPos;

            void main()
            {
                FragColor = texture(texture1, TexCoord);
                if (hideClose > 0.4) {
        float radius = 200.0;
        float fadeStart = 190.0;  // Start fading at 40 units
        float dist = distance(vertPos, camPos);

        // Ensure alpha is 1.0 if beyond radius, and smoothly fades inside
        float alphaFactor = clamp((dist - fadeStart) / (radius - fadeStart), 0.0, 1.0);
        FragColor.a *= alphaFactor;
        if(FragColor.a < 0.1) {
            discard;
        }
    }
            }
        )glsl",
        "lootDropGltfInstanceSHader"

    );

    static jl::ModelAndTextures basemodeltex = jl::ModelLoader::loadModel("resources/models/drop1.glb", false);

    glUseProgram(shader.shaderID);

    //glUniformMatrix4fv(glGetUniformLocation(gltfShader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
    //glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, scene->worldtex);


    glUniform1i(glGetUniformLocation(shader.shaderID, "texture1"), 0);


    auto camdir = camera.transform.direction;
    camdir.y = 0;
    camdir = glm::normalize(camdir);

    glm::vec3 posToRenderAt = camera.transform.position - (camdir * 0.3f);
    posToRenderAt.y = camera.transform.position.y - 0.5f;
    glUniform3f(glGetUniformLocation(shader.shaderID, "pos"), posToRenderAt.x, posToRenderAt.y, posToRenderAt.z);
    glUniform1f(glGetUniformLocation(shader.shaderID, "rot"), 0);
    glUniform1f(glGetUniformLocation(shader.shaderID, "hideClose"), 0.0f);
    glUniform3f(glGetUniformLocation(shader.shaderID, "camPos"),0.f, 0.f, 0.f);

    for(jl::ModelGLObjects &mglo : jp.modelGLObjects)
    {
        glBindVertexArray(mglo.vao);
        //Indent operations on this vertex array object
        glDrawElements(mglo.drawmode, mglo.indexcount, mglo.indextype, nullptr);

        glBindVertexArray(0);
    }

    for (auto & mglo : basemodeltex.modelGLObjects)
    {
        glBindVertexArray(mglo.vao);

    }

    auto view = reg.view<LootDrop, NPPositionComponent>();

    GLuint vao = 0;
    GLuint basevbo = 0;
    GLuint



}

