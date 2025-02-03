//
// Created by jack on 10/13/2024.
//

#ifndef BASICGLTFSHADER_H
#define BASICGLTFSHADER_H
#include "Shader.h"


jl::Shader getBasicShader();

inline jl::Shader getBasicShader()
{
    jl::Shader shader(
        R"glsl(
            #version 330 core
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec2 inTexCoord;
            layout(location = 2) in float inBrightness;
            layout(location = 3) in float isGrass;

            uniform float grassRedChange;

            mat4 getRotationMatrix(float xrot, float yrot, float zrot) {
                mat4 Rx = mat4(1.0, 0.0, 0.0, 0.0,
                               0.0, cos(xrot), -sin(xrot), 0.0,
                               0.0, sin(xrot), cos(xrot), 0.0,
                               0.0, 0.0, 0.0, 1.0);

                mat4 Ry = mat4(cos(yrot), 0.0, sin(yrot), 0.0,
                               0.0, 1.0, 0.0, 0.0,
                               -sin(yrot), 0.0, cos(yrot), 0.0,
                               0.0, 0.0, 0.0, 1.0);

                mat4 Rz = mat4(cos(zrot), -sin(zrot), 0.0, 0.0,
                               sin(zrot), cos(zrot), 0.0, 0.0,
                               0.0, 0.0, 1.0, 0.0,
                               0.0, 0.0, 0.0, 1.0);

                return Rz * Ry * Rx; // Note: The order might need to be adjusted based on your specific needs
            }
        uniform float rot;

            out vec2 TexCoord;

            uniform mat4 mvp;
            uniform vec3 pos;
            out float brightness;
            out vec3 ppos;
            out vec3 grassColor;

            void main()
            {

                mat4 rotmat = getRotationMatrix(0.0, 0.0, rot);
                vec4 rotposition = rotmat * vec4(inPosition, 1.0);
                ppos = inPosition;
                brightness = inBrightness;
                TexCoord = inTexCoord;
                gl_Position = mvp * vec4((rotposition.xyz + pos), 1.0);

                if(isGrass == 1.0) {

                    if(grassRedChange < 0.0) {
                        grassColor = vec3(0.0, 0.0, -0.2 * grassRedChange);
                    } else {
                        grassColor = vec3(grassRedChange, 0.0, 0.0);
                    }
                } else {
                    grassColor = vec3(0.0, 0.0, 0.0);
                }
            }
        )glsl",
        R"glsl(
            #version 330 core
            out vec4 FragColor;

            in vec2 TexCoord;
            in float brightness;
            uniform sampler2D texture1;
            uniform vec3 camPos;

            in vec3 ppos;
            in vec3 grassColor;
            float mDist(float x1, float y1, float x2, float y2) {
                return floor(abs(x2 - x1) + abs(y2-y1));
            }

            void main()
            {

                float distance = (mDist(ppos.x, ppos.z, camPos.x, camPos.z)/(15.0f*5.0f))/5.0f;
                vec4 fogColor = vec4(0.4, 0.75, 1.0, 1.0);
                vec4 tex = texture(texture1, TexCoord) + vec4(grassColor, 0.0);
                FragColor = vec4(tex.xyz * brightness, tex.w);
                FragColor = mix(FragColor, fogColor, min(1.0f, max(distance, 0.0f)));



            }
        )glsl",
        "basicGLTFShader"

        );
    return shader;
}





#endif //BASICGLTFSHADER_H
