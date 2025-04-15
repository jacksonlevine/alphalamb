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
            uniform vec3 offs;

            uniform float timeRendered;

            out float brightness;

            out vec3 ppos;
            out vec3 grassColor;
            out float timeRended;
            uniform vec3 camPos;
            uniform float worldCurveAmount;
            uniform float ambientBrightness;

            void main()
            {

                mat4 rotmat = getRotationMatrix(0.0, 0.0, rot);
                vec4 rotposition = rotmat * vec4(inPosition, 1.0);

                float occlvalues[4] = float[4](0.0, -0.3, -0.5, -0.7);
               uint ppp = floatBitsToUint(inBrightness);

                //// Unpack:
                uint block = (ppp >> 16u) & 0xFFFFu;     // upper 16 bits
                uint occlusion = (ppp >> 14u) & 0x3u;    // bits 14–15
                uint ambient = ppp & 0x3FFFu;            // lower 14 bits


                brightness = max(0.2, min(1.0, (float(block)/16.0) + ((float(ambient)/16.0) * ambientBrightness)) + occlvalues[occlusion]);
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
            }
        )glsl",
        R"glsl(#version 450 core
            out vec4 FragColor;

            in vec2 TexCoord;
            in float brightness;
            uniform sampler2D texture1;
            uniform sampler3D lut;
            uniform vec3 camPos;
            in float timeRended;
            uniform float renderDistance;
            uniform float ambientBrightness;
            uniform vec3 fogCol;
            uniform float overridingDewyFogFactor;
            uniform float dewyFogFactor;


            uniform float underwater;

            in vec3 ppos;
            in vec3 grassColor;
            float mDist(float x1, float y1, float x2, float y2) {
                return floor(abs(x2 - x1) + abs(y2-y1));
            }

            void main()
            {
float clearRadius = 25.0;     // Fog-free radius around the player
float maxFogHeight = 120.0;   // Fog starts at y=120
float minFogHeight = 20.0;    // Fog reaches max at y=20
float fogMaxDistance = renderDistance * 16.0; // Fog fades in near render distance

// Calculate horizontal (XZ) distance from camera
float horizDist = length(ppos - camPos);

// Calculate vertical distance (height fog)
float heightFactor = smoothstep(maxFogHeight, minFogHeight, ppos.y);

// Distance fog (fades in near render distance)
float distanceFactor = smoothstep(fogMaxDistance * 0.7, fogMaxDistance, horizDist);

// Fog-free radius around the player
float clearFactor = smoothstep(clearRadius - 23.0, clearRadius, horizDist);

// Calculate fog intensity at camera height
float camHeightFog = smoothstep(maxFogHeight, minFogHeight, camPos.y) * overridingDewyFogFactor;

// Combine fog factors
float fogFactor = max(heightFactor, distanceFactor);

// Apply camera height fog to everything
fogFactor = max(fogFactor, camHeightFog);

// Override fog with clearFactor (fog-free radius)
fogFactor *= clearFactor;


float dff = max(overridingDewyFogFactor, dewyFogFactor);

fogFactor *= dff;

// Apply fog to the final color
vec4 fogColor = vec4(fogCol.xyz, 1.0);
if(underwater > 0.5f) {
    fogColor = vec4(0.2, 0.2, 0.8, 1.0) * ambientBrightness;
    fogFactor = smoothstep(1.0, 15.0, horizDist);

}
vec4 tex = texture(texture1, TexCoord) + vec4(grassColor * 0.3, 0.0);
FragColor = mix(vec4((tex * brightness).xyz, tex.a), fogColor, fogFactor);
                vec3 lutCoords = clamp(FragColor.xyz, 0.0, 1.0);
                lutCoords = lutCoords * (63.0/64.0) + (0.5/64.0);
                vec4 lutTex = texture(lut, lutCoords);

                FragColor = vec4(lutTex.xyz, FragColor.a * timeRended);


                if(FragColor.a < 0.1f) {
                    discard;
                }
                ////Fresnel effect on semi-transparent stuff right here
                //if(FragColor.a < 1.0) {
                //    FragColor.a += distance*100.5f;
                //}


            }
        )glsl",
        "basicGLTFShader"

        );
    return shader;
}