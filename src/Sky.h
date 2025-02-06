//
// Created by jack on 1/29/2025.
//

#ifndef SKY_H
#define SKY_H
#include "Camera.h"
#include "Shader.h"

inline void drawSky(glm::vec4 top, glm::vec4 bot, float ambBrightness, jl::Camera* camera, GLuint lut) {
    // Sky
    static GLuint skyVao = 0;
    if (skyVao == 0)
    {
        glGenVertexArrays(1, &skyVao);
        
    }

    static GLuint skyshader = 0;
    if (skyshader == 0)
    {
        jl::Shader ss(
            R"glsl(
                #version 430 core
                out vec2 v_uv;
                uniform float cpitch;
                void main()
                {
                    uint idx = gl_VertexID;
                    gl_Position = vec4((idx & 1), (idx >> 1), 0.0, 0.5) * 4.0 - 1.0;
                    v_uv = vec2(gl_Position.xy + 1.0 + (cpitch / 62));
                }


            )glsl",
            R"glsl(
                #version 430 core
                uniform vec4 top_color;
                uniform vec4 bot_color;
                uniform float brightMult;
                uniform float sunrise;
                uniform float sunset;
                uniform vec3 camDir;
                uniform sampler3D lut;
                in vec2 v_uv;

                out vec4 frag_color;

                float similarity(vec3 dir1, vec3 dir2) {
                    return (dot(normalize(dir1), normalize(dir2)) + 1.0) * 0.5;
                }

                void main()
                {
                    vec3 east = vec3(0, 0, 1);
                    vec3 west = vec3(0, 0, -1);

                    vec4 sunsetcol = vec4(1.0f, 0.651f, 0.0f, 1.0f);
                    vec4 sunrisecol = vec4(1.0f, 0.651f, 0.0f, 1.0f);



                    vec4 botColor = mix(bot_color * vec4(brightMult, brightMult, brightMult, 1.0f), sunrisecol, (similarity(camDir, east)) * sunrise);
                    botColor = mix(botColor, sunsetcol, (similarity(camDir, west)) * sunset);
                    frag_color = mix(botColor, top_color * vec4(brightMult, brightMult, brightMult, 1.0f), max(min(pow(v_uv.y-0.4, 1.0), 1.0), 0.0));

                    vec3 lutCoords = clamp(frag_color.xyz, 0.0, 1.0);
                    lutCoords = lutCoords * (63.0/64.0) + (0.5/64.0);
                    vec4 lutTex = texture(lut, lutCoords);
                    frag_color = vec4(lutTex.xyz, frag_color.a);

                }

            )glsl",
            "skyShader"
            );
        skyshader = ss.shaderID;
    }
    
    glBindVertexArray(skyVao);
    glUseProgram(skyshader);
    glDisable(GL_DEPTH_TEST);

    static GLuint lutLoc = glGetUniformLocation(skyshader, "lut");

    static int T_C_LOC = -1;
    static int B_C_LOC = 0;
    static int C_P_LOC = 0;
    static int A_B_LOC = 0;
    static int S_S_LOC = 0;
    static int S_R_LOC = 0;
    static int C_D_LOC = 0;

    if (T_C_LOC == -1) {
        T_C_LOC = glGetUniformLocation(
            skyshader,
            "top_color"
        );
        B_C_LOC = glGetUniformLocation(
            skyshader,
            "bot_color"
        );
        C_P_LOC = glGetUniformLocation(
            skyshader,
            "cpitch"
        );
        A_B_LOC = glGetUniformLocation(
            skyshader,
            "brightMult"
        );
        S_S_LOC = glGetUniformLocation(
            skyshader,
            "sunset"
        );
        S_R_LOC = glGetUniformLocation(
            skyshader,
            "sunrise"
        );
        C_D_LOC = glGetUniformLocation(
            skyshader,
            "camDir"
        );
    }


    glUniform1f(C_P_LOC, camera->transform.pitch);
    glUniform3f(
        C_D_LOC,
        camera->transform.direction.x,
        camera->transform.direction.y,
        camera->transform.direction.z
    );

    glUniform4f(T_C_LOC, top.x, top.y, top.z, top.w);
    glUniform4f(B_C_LOC, bot.x, bot.y, bot.z, bot.w);

    glUniform1f(A_B_LOC, ambBrightness);
    glUniform1f(S_S_LOC, 0.0f);
    glUniform1f(S_R_LOC, 0.0f);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(lutLoc, 1);


    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

#endif //SKY_H
