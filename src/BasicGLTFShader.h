
#ifndef BASICGLTFSHADER_H
#define BASICGLTFSHADER_H
#include "Shader.h"


jl::Shader getBasicGLTFShader();

inline jl::Shader getBasicGLTFShader()
{
    jl::Shader shader(
        R"glsl(
            #version 330 core
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec2 inTexCoord;

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

            void main()
            {

                mat4 rotmat = getRotationMatrix(0.0, 0.0, rot);
                vec4 rotposition = rotmat * vec4(inPosition, 1.0);

                TexCoord = inTexCoord;
                gl_Position = mvp * vec4((rotposition.xyz + pos), 1.0);
            }
        )glsl",
        R"glsl(
            #version 330 core
            out vec4 FragColor;

            in vec2 TexCoord;

            uniform sampler2D texture1;

            void main()
            {
                FragColor = texture(texture1, TexCoord);
            }
        )glsl",
        "basicGLTFShader"

        );
    return shader;
}





#endif //BASICGLTFSHADER_H
