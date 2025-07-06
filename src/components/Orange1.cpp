//
// Created by jack on 6/12/2025.
//

#include "Orange1.h"

#include <complex.h>
#include <complex.h>

#include "NPPositionComponent.h"
#include "SoundSource.h"
#include "../Client.h"
#include "../ModelLoader.h"
#include "../Shader.h"
#include "../Scene.h"
#include "../PhysXStuff.h"
#include "../TextureFace.h"
#include "../world/gizmos/ParticlesGizmo.h"


std::array<glm::vec3, 7> getJungleCampOffsetsForWorldXZ(FastNoiseLite& noise, const TwoIntTup& wxz)
{
    std::array<glm::vec3, 7> offsets;
    for (int i = 0; i < 7; i++)
    {
        auto ns1 = std::min(-15.f, std::max(15.f, noise.GetNoise(wxz.x * i * 75.f, wxz.z * i * 75.f) * 25.0f));
        auto ns3 = std::min(-15.f, std::max(15.f, noise.GetNoise((wxz.x+1)* i * 75.f, wxz.z * i * 75.f) * 25.0f));
        offsets[i] = glm::vec3(ns1,0.f,ns3);
    }
    return offsets;
}


void renderOrange1Guys(entt::registry& reg, Scene* scene, float deltaTime)
{

    struct Orange1PhysicsBody
    {
        PxRigidStatic* body = nullptr;
        Orange1PhysicsBody() = default;
        explicit Orange1PhysicsBody(const glm::vec3& position)
        {
            const PxMaterial* material = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
            const PxSphereGeometry sphereGeom(0.25f);
            PxRigidStatic* sphere = gPhysics->createRigidStatic(PxTransform(PxVec3(position.x, position.y, position.z)));
            PxShape* shape = gPhysics->createShape(sphereGeom, *material);

            setCollisionFilter(shape, static_cast<uint32_t>(CollisionGroup::GROUP_ANIMAL), static_cast<uint32_t>(CollisionGroup::GROUP_WORLD));
            //
            // if (shape)
            // {
            //
            //     // Set the collision filtering
            //     PxFilterData filterData;
            //     filterData.word0 = 1;  // Set to group 1
            //     shape->setSimulationFilterData(filterData);
            //     shape->setQueryFilterData(filterData);
            // }



            sphere->attachShape(*shape);
            shape->release(); //Release our reference, actor holds it now
            //PxRigidBodyExt::updateMassAndInertia(*sphere, 1.0f);
            gScene->addActor(*sphere);
            body = sphere;
        }
        ~Orange1PhysicsBody()
        {
            if (body)
            {
                body->release();
                body = nullptr;
            }
        }
        Orange1PhysicsBody& operator=(Orange1PhysicsBody& other) = delete;
        Orange1PhysicsBody(Orange1PhysicsBody& other) = delete;
        Orange1PhysicsBody(Orange1PhysicsBody&& other)
        {
            body = other.body;
            other.body = nullptr;
        }
        Orange1PhysicsBody& operator=(Orange1PhysicsBody&& other)
        {
            body = other.body;
            other.body = nullptr;
            return *this;
        }
    };

    static std::unordered_map<TwoIntTup, std::vector<entt::entity>, TwoIntTupHash> guysatspots;



    static std::unordered_map<entt::entity, Orange1PhysicsBody> physicsbodies;



    struct Orange1DisplayInstance
    {
        glm::vec3 pos;
        glm::vec4 quat;
        glm::vec2 uvoffset;
    };



    static std::vector<Orange1DisplayInstance> orange1DisplayInstances;


    static jl::Shader shader(
        R"glsl(
            #version 330 core
            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec2 inTexCoord;

            layout(location = 2) in vec3 instancePos;
            layout(location = 3) in vec4 instanceQuat;

            layout(location = 4) in vec3 inNormal; // added on after excuse the weird ordering :>)

            layout(location = 5) in vec2 instanceUVOffset;

            out vec2 TexCoord;
            uniform float time;
            uniform vec3 camDir;
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

            float getDirectionFactorToDirection(vec3 direction, vec3 toDirection) {
                return 1.0 - ( dot(normalize(direction), toDirection) * 0.5 + 0.5);
            }

uint hash(uint x, uint seed) {
    const uint m = 0x5bd1e995U;
    uint hash = seed;
    // process input
    uint k = x;
    k *= m;
    k ^= k >> 24;
    k *= m;
    hash *= m;
    hash ^= k;
    // some final mixing
    hash ^= hash >> 13;
    hash *= m;
    hash ^= hash >> 15;
    return hash;
}

// implementation of MurmurHash (https://sites.google.com/site/murmurhash/) for a
// 3-dimensional unsigned integer input vector.

uint hash(uvec3 x, uint seed){
    const uint m = 0x5bd1e995U;
    uint hash = seed;
    // process first vector element
    uint k = x.x;
    k *= m;
    k ^= k >> 24;
    k *= m;
    hash *= m;
    hash ^= k;
    // process second vector element
    k = x.y;
    k *= m;
    k ^= k >> 24;
    k *= m;
    hash *= m;
    hash ^= k;
    // process third vector element
    k = x.z;
    k *= m;
    k ^= k >> 24;
    k *= m;
    hash *= m;
    hash ^= k;
	// some final mixing
    hash ^= hash >> 13;
    hash *= m;
    hash ^= hash >> 15;
    return hash;
}


vec3 gradientDirection(uint hash) {
    switch (int(hash) & 15) { // look at the last four bits to pick a gradient direction
    case 0:
        return vec3(1, 1, 0);
    case 1:
        return vec3(-1, 1, 0);
    case 2:
        return vec3(1, -1, 0);
    case 3:
        return vec3(-1, -1, 0);
    case 4:
        return vec3(1, 0, 1);
    case 5:
        return vec3(-1, 0, 1);
    case 6:
        return vec3(1, 0, -1);
    case 7:
        return vec3(-1, 0, -1);
    case 8:
        return vec3(0, 1, 1);
    case 9:
        return vec3(0, -1, 1);
    case 10:
        return vec3(0, 1, -1);
    case 11:
        return vec3(0, -1, -1);
    case 12:
        return vec3(1, 1, 0);
    case 13:
        return vec3(-1, 1, 0);
    case 14:
        return vec3(0, -1, 1);
    case 15:
        return vec3(0, -1, -1);
    }
}

float interpolate(float value1, float value2, float value3, float value4, float value5, float value6, float value7, float value8, vec3 t) {
    return mix(
        mix(mix(value1, value2, t.x), mix(value3, value4, t.x), t.y),
        mix(mix(value5, value6, t.x), mix(value7, value8, t.x), t.y),
        t.z
    );
}

vec3 fade(vec3 t) {
    // 6t^5 - 15t^4 + 10t^3
	return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float perlinNoise(vec3 position, uint seed) {
    vec3 floorPosition = floor(position);
    vec3 fractPosition = position - floorPosition;
    uvec3 cellCoordinates = uvec3(floorPosition);
    float value1 = dot(gradientDirection(hash(cellCoordinates, seed)), fractPosition);
    float value2 = dot(gradientDirection(hash((cellCoordinates + uvec3(1, 0, 0)), seed)), fractPosition - vec3(1, 0, 0));
    float value3 = dot(gradientDirection(hash((cellCoordinates + uvec3(0, 1, 0)), seed)), fractPosition - vec3(0, 1, 0));
    float value4 = dot(gradientDirection(hash((cellCoordinates + uvec3(1, 1, 0)), seed)), fractPosition - vec3(1, 1, 0));
    float value5 = dot(gradientDirection(hash((cellCoordinates + uvec3(0, 0, 1)), seed)), fractPosition - vec3(0, 0, 1));
    float value6 = dot(gradientDirection(hash((cellCoordinates + uvec3(1, 0, 1)), seed)), fractPosition - vec3(1, 0, 1));
    float value7 = dot(gradientDirection(hash((cellCoordinates + uvec3(0, 1, 1)), seed)), fractPosition - vec3(0, 1, 1));
    float value8 = dot(gradientDirection(hash((cellCoordinates + uvec3(1, 1, 1)), seed)), fractPosition - vec3(1, 1, 1));
    return interpolate(value1, value2, value3, value4, value5, value6, value7, value8, fade(fractPosition));
}

            void main()
            {

                mat3 rotMatrix = quatToMat3(instanceQuat);

                vec3 rotatedPosition = rotMatrix * inPosition;
                vec3 rotatedNorm = rotMatrix * inNormal;

                vec4 worldPosition = vec4(rotatedPosition + instancePos, 1.0);

                gl_Position = mvp * worldPosition;
                float perlinFactor = perlinNoise(worldPosition.xyz + vec3(time, 0.0, time), uint(0));
                vbright = getDirectionFactorToDirection(rotatedNorm, normalize(cross(vec3(0.0, 1.0, 0.0),  camDir))) + perlinFactor;

                TexCoord = inTexCoord + vec2(instanceUVOffset.x, instanceUVOffset.y) ;
                vertPos = worldPosition.xyz;
            }
        )glsl",
        R"glsl(
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
        )glsl",
        "orange1GltfInstanceShader"

    );



    static const jl::ModelAndTextures basemodeltex = jl::loadModel<true>("resources/models/orange1.glb", false);


    glBindVertexArray(basemodeltex.modelGLObjects.at(0).vao);

    glUseProgram(shader.shaderID);

    //glUniformMatrix4fv(glGetUniformLocation(gltfShader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, basemodeltex.texids.at(0));


    glUniform1i(glGetUniformLocation(shader.shaderID, "texture1"), 0);

    const auto & camera = scene->our<jl::Camera>();

    glUniform1f(glGetUniformLocation(shader.shaderID, "hideClose"), 0.0f);
    glUniform3f(glGetUniformLocation(shader.shaderID, "camPos"),0.f, 0.f, 0.f);
    glUniform3f(glGetUniformLocation(shader.shaderID, "camDir"),camera.transform.direction.x, camera.transform.direction.y, camera.transform.direction.z);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
    glUniform1f(glGetUniformLocation(shader.shaderID, "time"), glfwGetTime());
    const auto view = reg.view<Orange1, NPPositionComponent>();
    orange1DisplayInstances.clear();
    for (auto entity : view)
    {
        //std::cout << "entity: "  << (int)entity << std::endl;
        auto & pos = view.get<NPPositionComponent>(entity);
        glm::vec4 quat(0.0f, 0.0f, 0.0f, 1.0f); // Default identity quaternion

        auto orange1 = view.get<Orange1>(entity);

        const auto oldblockspot = TwoIntTup(glm::floor(pos.position.x), glm::floor(pos.position.z));

        //Remove old guysatspots entry if there
        if(guysatspots.contains(oldblockspot))
        {
            auto & there = guysatspots.at(oldblockspot);
            auto it = std::find(there.begin(), there.end(), entity);
            if(it != there.end())
            {
                there.erase(it);
            }
            if(there.empty())
            {
                guysatspots.erase(oldblockspot);
            }
        }

        //Update item to physics body spot
        if(physicsbodies.contains(entity))
        {
            const auto pose = physicsbodies.at(entity).body->getGlobalPose();
            const auto newpos = pose.p;
            const auto newquat = pose.q;
            pos.position = glm::vec3(newpos.x, newpos.y, newpos.z);
            quat = glm::vec4(newquat.x, newquat.y, newquat.z, newquat.w);

        } else
        {
            physicsbodies.insert_or_assign(entity, Orange1PhysicsBody(pos.position));
        }


        //Re-add to guysatspots
        const auto newblockspot = TwoIntTup(glm::floor(pos.position.x), glm::floor(pos.position.z));
        if(!guysatspots.contains(newblockspot))
        {
            guysatspots.insert({newblockspot, std::vector<entt::entity>{ {entity} }});
        } else
        {
            auto & bp = guysatspots.at(newblockspot);

            if(std::find(bp.begin(), bp.end(), entity) == bp.end())
            {
                bp.push_back(entity);
            }
        }

        orange1DisplayInstances.emplace_back(pos.position, quat, glm::vec2(0,0));
    }

    static GLuint instancevbo = 0;

    const auto  &mglo = basemodeltex.modelGLObjects.at(0);

        glBindVertexArray(mglo.vao);

        if (instancevbo == 0 )
        {
            glGenBuffers(1, &instancevbo);
        }

            glBindBuffer(GL_ARRAY_BUFFER, instancevbo);

            glBufferData(GL_ARRAY_BUFFER, sizeof(Orange1DisplayInstance) * orange1DisplayInstances.size(), orange1DisplayInstances.data(), GL_DYNAMIC_DRAW);
            // Position attribute (location 2)
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Orange1DisplayInstance), (GLvoid*)0);
            glEnableVertexAttribArray(2);
            glVertexAttribDivisor(2, 1);

            // Quaternion attribute (location 3)
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Orange1DisplayInstance), (GLvoid*)(3 * sizeof(float)));
            glEnableVertexAttribArray(3);
            glVertexAttribDivisor(3, 1);

            // UVOFFSET of instance (location 5)
            glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Orange1DisplayInstance), (GLvoid*)(7 * sizeof(float)));
            glEnableVertexAttribArray(5);
            glVertexAttribDivisor(5, 1);


        glDrawElementsInstanced(mglo.drawmode, mglo.indexcount, mglo.indextype, nullptr, orange1DisplayInstances.size());


        std::erase_if(physicsbodies, [&reg](const auto& pair) {
            return !reg.valid(pair.first);
        });


    const auto & campos = scene->our<jl::Camera>().transform.position;
    auto blockspot = TwoIntTup(glm::floor(campos.x), glm::floor(campos.z));
    auto & inv = scene->our<InventoryComponent>();
    if(guysatspots.contains(blockspot))
    {
        auto & bp = guysatspots.at(blockspot);
        std::erase_if(bp, [&reg](entt::entity ent)
        {
            return !reg.valid(ent);
        });

        // for(auto ent : bp)
        // {
        //     auto & drop = reg.get<Orange1>(ent);
        //     if(!inv.full(drop))
        //     {
        //         auto thepos = reg.get<NPPositionComponent>(ent).position;
        //
        //         // if(glm::distance(thepos, campos) < 1.5f)
        //         // {
        //         //     pushToMainToNetworkQueue(PickUpOrange1Drop{scene->myPlayerIndex, ent});
        //         // }
        //
        //     }
        // }
    }
}


void attackPlayers(entt::registry &reg, float deltaTime, Scene* scene)
{
    auto orangeguysview = reg.view<Orange1, NPPositionComponent, SoundSource<Orange1Hisser>>();
    auto playersview = reg.view<PlayerComp, jl::Camera, Controls>();

    static std::unordered_map<entt::entity, float> timers = {};

    for (auto orangeguyentity : orangeguysview)
    {

        if (!timers.contains(orangeguyentity))
        {
            timers.insert({orangeguyentity, 0.0f});
        } else
        {
            auto &f = timers.at(orangeguyentity);
            if (f < 1.0f)
            {
                f += deltaTime;
            } else
            {
                f = 0;
                auto & ogp = orangeguysview.get<NPPositionComponent>(orangeguyentity).position;
                auto & ss = orangeguysview.get<SoundSource<Orange1Hisser>>(orangeguyentity);


                for (auto entity: playersview)
                {
                    //Aim for just over their head
                    auto & cam = playersview.get<jl::Camera>(entity);
                    auto ppos = cam.transform.position + glm::vec3(0.f, 1.f, 0.f);
                    auto &cont = playersview.get<Controls>(entity);

                    auto dist = glm::distance(ppos, ogp);

                    if (cont.anyMovement() && glm::distance(ogp, ppos + cam.transform.direction) >= dist )
                    {
                        ppos += cam.transform.direction;
                    }


                    //std::cout << "Dist: " << dist << std::endl;
                    if (dist < 10)
                    {
                        auto dir = betterNormalize(ppos - (ogp + glm::vec3(0.f, 5.f, 0.f)));
                        //Spawn a Dart
                        ss.play(sounds.at((int)SoundBuffers::DARTSHOOT));
                        scene->particles->particleBurst(ogp + glm::vec3(0.f, 5.f, 0.f),
                                                            5, (MaterialName)JETPACK_PARTICLE_BLOCK, 0.8, 8.0f);
                        DGMessage msg = SpawnGuy{GuyType::DART1, entt::null, ogp + glm::vec3(0.f, 5.f, 0.f),  dir, 0.5f};
                        pushToMainToNetworkQueue(msg);
                    }
                }
            }
        }

    }



}

