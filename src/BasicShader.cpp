//
// Created by jack on 3/23/2025.
//
#include "BasicShader.h"


jl::Shader getBasicShader()
{
    jl::Shader shader(
        R"glsl(#version 450 core
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec2 inTexCoord;
            layout(location = 2) in float inBrightness;
            layout(location = 3) in float isGrass;

            uniform float grassRedChange;
            uniform float scale;

float closenessToNearestMultiple(float value, float N, float radius) {
    float remainder = mod(value, N);
    float distance = min(remainder, N - remainder);
    float normalized = 1.0 - (distance / (N * 0.5));
    return clamp((normalized - (1.0 - radius)) / radius, 0.0, 1.0);
}
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
            out vec3 brightness;
            out vec3 ppos;
            out vec3 grassColor;
            out float timeRended;
            out float fogFactor; // Moved to vertex shader
            out vec4 fogColor;   // Moved to vertex shader

            uniform mat4 mvp;
            uniform vec3 pos;
            uniform vec3 offs;
            uniform float timeRendered;
            uniform vec3 camPos;
            uniform float worldCurveAmount;
            uniform float ambientBrightness;
            uniform float renderDistance;
            uniform float overridingDewyFogFactor;
            uniform float dewyFogFactor;
            uniform vec3 fogCol;
            uniform float underwater;
            uniform float dewyFogAmount;

out float closenessToJungleCamp;


            float mDist(float x1, float y1, float x2, float y2) {
                return floor(abs(x2 - x1) + abs(y2-y1));
            }

            void main()
            {
                mat4 rotmat = getRotationMatrix(0.0, 0.0, rot);
                vec4 rotposition = rotmat * vec4(inPosition, 1.0);

                float occlvalues[4] = float[4](0.0, -0.3, -0.5, -0.7);
                uint ppp = floatBitsToUint(inBrightness);

                uint blockRaw   = (ppp >> 16u) & 0xFFFFu; // Get the upper 16 bits for the block (12-bit RGB)
                uint ambientRaw = ppp & 0x3FFFu;          // Get the lower 14 bits for the ambient (12-bit RGB)

                uint occlusion = (ppp >> 14u) & 0x3u;    // bits 14–15

                ambientRaw &= 0x0FFFu;

                uint blockr = (blockRaw >> 8u) & 0xFu;
                uint blockg = (blockRaw >> 4u) & 0xFu;
                uint blockb = blockRaw & 0xFu;

                uint ambientr = (ambientRaw >> 8u) & 0xFu;
                uint ambientg = (ambientRaw >> 4u) & 0xFu;
                uint ambientb = ambientRaw & 0xFu;

                brightness =
                vec3(
                max(0.2, min(1.0, (float(blockr)/12.0) + ((float(ambientr)/12.0) * ambientBrightness)) + occlvalues[occlusion]),
                max(0.2, min(1.0, (float(blockg)/12.0) + ((float(ambientg)/12.0) * ambientBrightness)) + occlvalues[occlusion]),
                max(0.2, min(1.0, (float(blockb)/12.0) + ((float(ambientb)/12.0) * ambientBrightness)) + occlvalues[occlusion])
                );

                TexCoord = inTexCoord;

                float fadeInProgress = min(1.0, timeRendered*2.5f);

                timeRended = fadeInProgress;
                vec3 rotpos = rotposition.xyz  + offs;
                vec3 scaledPos = vec3(rotpos.x * scale, rotpos.y * scale, rotpos.z * scale);
                vec3 pos2 = (scaledPos.xyz + pos + vec3(0.0, 0.0, 0.0));

                ppos = pos2;

                float distFromCP = distance(camPos.xz, pos2.xz);
                pos2.y -= pow(distFromCP* 0.025, 2) * worldCurveAmount;
                gl_Position = mvp * vec4(pos2, 1.0);
                if(isGrass == 1.0) {
                    if(grassRedChange < 0.0) {
                        grassColor = vec3(0.0, 0.0, -0.2 * grassRedChange);
                    } else {
                        grassColor = vec3(grassRedChange, 0.0, 0.0);
                    }
                } else {
                    grassColor = vec3(0.0, 0.0, 0.0);
                }

                // Fog calculations moved from fragment shader
                float clearRadius = 25.0;     // Fog-free radius around the player
                float maxFogHeight = 120.0;   // Fog starts at y=120
                float minFogHeight = 20.0;    // Fog reaches max at y=20
                float fogMaxDistance = renderDistance * 16.0; // Fog fades in near render distance

                // Calculate horizontal (XZ) distance from camera
                float horizDist = mDist(ppos.x, ppos.z, camPos.x, camPos.z);

                // Calculate vertical distance (height fog)
                float heightFactor = smoothstep(maxFogHeight, minFogHeight, ppos.y);

                // Distance fog (fades in near render distance)
                float distanceFactor = smoothstep(fogMaxDistance * 0.7, fogMaxDistance, horizDist);

                // Fog-free radius around the player
                float clearFactor = smoothstep(clearRadius - 23.0, clearRadius, horizDist);

                // Calculate fog intensity at camera height
                float camHeightFog = smoothstep(maxFogHeight, minFogHeight, camPos.y) * overridingDewyFogFactor;

                // Combine fog factors
                fogFactor = max(heightFactor, distanceFactor);

                // Apply camera height fog to everything
                fogFactor = max(fogFactor, camHeightFog);

                // Override fog with clearFactor (fog-free radius)
                fogFactor *= clearFactor;

                float dff = max(overridingDewyFogFactor, dewyFogFactor) * dewyFogAmount;
                fogFactor *= dff;

                // Set fog color
                fogColor = vec4(fogCol.xyz  * ambientBrightness, 1.0);
                if(underwater > 0.5f) {
                    fogColor = vec4(vec3(0.2, 0.2, 0.8) * ambientBrightness, 1.0);
                    fogFactor = smoothstep(1.0, 15.0, horizDist);
                }

closenessToJungleCamp = closenessToNearestMultiple(pos2.x, 1250.0, 0.1f) * closenessToNearestMultiple(pos2.z, 1250.0, 0.1f);
            }
        )glsl",
        R"glsl(#version 450 core
            out vec4 FragColor;
            in float closenessToJungleCamp;
            in vec2 TexCoord;
            in vec3 brightness;
            in vec3 ppos;
            in vec3 grassColor;
            in float timeRended;
            in float fogFactor; // Moved from vertex shader
            in vec4 fogColor;   // Moved from vertex shader

            uniform sampler2D texture1;
            uniform sampler3D lut;
            uniform vec3 camPos;
            uniform float renderDistance;
            uniform float ambientBrightness;
            uniform float overridingDewyFogFactor;
            uniform float dewyFogFactor;
            uniform vec3 fogCol;
            uniform float underwater;

            void main()
            {
                vec4 tex = texture(texture1, TexCoord) + vec4(grassColor * 0.3, 0.0);
                FragColor = mix(vec4(vec3(tex.r * brightness.r, tex.g * brightness.g, tex.b * brightness.b), tex.a), fogColor, fogFactor);
                vec3 lutCoords = clamp(FragColor.xyz, 0.0, 1.0);
                lutCoords = lutCoords * (63.0/64.0) + (0.5/64.0);
                vec4 lutTex = texture(lut, lutCoords);

                FragColor = vec4(lutTex.xyz, FragColor.a * timeRended);
                FragColor.b += closenessToJungleCamp;

                if(FragColor.a < 0.1f) {
                    discard;
                }
            }
        )glsl",
        "basicGLTFShader"
    );
    return shader;
}