
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
