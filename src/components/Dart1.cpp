//
// Created by jack on 6/16/2025.
//

#include "Dart1.h"

#include "../PhysXStuff.h"
#include "../CollisionCage.h"
#include "../ModelLoader.h"
#include "../Scene.h"
#include "../Client.h"
#include "../world/gizmos/ParticlesGizmo.h"
#include "../InstancedThingyTemplate.h"

constexpr const char DART1MODELPATH[] = "resources/models/dart1.glb";

constexpr const char DART1VERTSHAD[] = R"glsl(
            #version 330 core
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec2 inTexCoord;

            layout(location = 2) in vec3 instancePos;
            layout(location = 3) in vec4 instanceQuat;

            layout(location = 4) in vec3 inNormal; // added on after excuse the weird ordering :>)

            layout(location = 5) in vec2 instanceUVOffset;

            out vec2 TexCoord;
            uniform float time;
            uniform mat4 mvp;
            out vec3 vertPos;
            out float vbright;
            mat3 quatToMat3(vec4 q) {
                float x = q.x, y = q.y, z = q.z, w = q.w;
                float x2 = x + x, y2 = y + y, z2 = z + z;
                float xx = x * x2, xy = x * y2, xz = x * z2;
                float yy = y * y2, yz = y * z2, zz = z * z2;
                float wx = w * x2, wy = w * y2, wz = w * z2;

                return mat3(
                    1.0 - (yy + zz), xy + wz, xz - wy,
                    xy - wz, 1.0 - (xx + zz), yz + wx,
                    xz + wy, yz - wx, 1.0 - (xx + yy)
                );
            }

            float getDirectionFactor(vec3 direction) {
                return 1.0 - ( dot(normalize(direction), vec3(0.0, -1.0, 0.0)) * 0.5 + 0.5);
            }
            void main()
            {

                mat3 rotMatrix = quatToMat3(instanceQuat);

                vec3 rotatedPosition = rotMatrix * inPosition;
                vec3 rotatedNorm = rotMatrix * inNormal;

                vec4 worldPosition = vec4(rotatedPosition + instancePos, 1.0);

                gl_Position = mvp * worldPosition;
                vbright = getDirectionFactor(rotatedNorm);

                TexCoord = inTexCoord + vec2(instanceUVOffset.x, instanceUVOffset.y) ;
                vertPos = worldPosition.xyz;
            }
        )glsl";
constexpr const char DART1FRAGSHAD[] = R"glsl(
            #version 330 core
            out vec4 FragColor;

            uniform float hideClose;

            in vec2 TexCoord;
            in vec3 vertPos;
            in float vbright;

            uniform sampler2D texture1;
            uniform vec3 camPos;

            void main()
            {
                FragColor = texture(texture1, TexCoord);

                FragColor = vec4(FragColor.x*vbright, FragColor.y*vbright, FragColor.z*vbright, FragColor.a);
                if(FragColor.a < 0.1) {
                    discard;
                }

            }
        )glsl";


void renderDart1s(entt::registry& reg, Scene* scene, float deltaTime)
{
    static BehaviorFunction func = [](Scene* scene, float deltaTime, entt::registry& reg,
                                      std::unordered_map<TwoIntTup, std::vector<entt::entity>, TwoIntTupHash>&
                                      dartsatspots)
    {
        const auto& campos = scene->our<jl::Camera>().transform.position;
        auto blockspot = TwoIntTup(glm::floor(campos.x), glm::floor(campos.z));
        if (dartsatspots.contains(blockspot))
        {
            auto& bp = dartsatspots.at(blockspot);
            std::erase_if(bp, [&reg](entt::entity ent)
            {
                return !(reg.valid(ent) && reg.all_of<Dart1>(ent));
            });

            for (auto ent : bp)
            {
                if (reg.valid(ent) && reg.all_of<Dart1>(ent))
                {
                    auto& dart = reg.get<Dart1>(ent);

                    auto thepos = reg.get<NPPositionComponent>(ent).position;

                    if (glm::distance(thepos, campos) < 1.5f)
                    {
                        scene->particles->particleBurst(thepos,
                                                        10, (MaterialName)JETPACK_PARTICLE_BLOCK, 0.8, 4.0f);
                        pushToMainToNetworkQueue(DealDamage{
                            .playerIndex = scene->myPlayerIndex, .amount = 1.75f, .projectileToDelete = ent,
                            .damager = ent
                        });
                    }
                }
            }
        }
    };

    renderAndBehaveInstancedThingy<Dart1, DART1VERTSHAD, DART1FRAGSHAD, DART1MODELPATH, GROUP_PARTICLE, GROUP_WORLD>(reg, scene, deltaTime, func);
}
