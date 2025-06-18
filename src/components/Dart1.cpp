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

void renderDart1s(entt::registry& reg, Scene* scene, float deltaTime)
{

    struct DartPhysicsBody
    {
        PxRigidDynamic* body = nullptr;
        CollisionCage<2> collisionCage = {};
        DartPhysicsBody() = default;
        explicit DartPhysicsBody(const glm::vec3& position, const glm::vec3& direction)
        {
            const PxMaterial* material = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
            const PxSphereGeometry sphereGeom(0.25f);

            PxVec3 forward = PxVec3(direction.x, direction.y, direction.z);
            PxVec3 worldUp = PxVec3(0, 1, 0);

            // Compute right and adjusted up vectors
            PxVec3 right = worldUp.cross(forward).getNormalized();
            PxVec3 up = forward.cross(right);

            // Create 3x3 rotation matrix from axes
            PxMat33 rotation(right, up, forward);  // columns: X=right, Y=up, Z=forward


            PxRigidDynamic* sphere = gPhysics->createRigidDynamic(PxTransform(PxVec3(position.x, position.y, position.z), PxQuat(rotation)));
            PxShape* shape = gPhysics->createShape(sphereGeom, *material);
            sphere->attachShape(*shape);
            shape->release(); //Release our reference, actor holds it now
            sphere->setLinearVelocity(PxVec3(direction.x, direction.y, direction.z) * 20.0f);
            sphere->setMass(5.f);
            gScene->addActor(*sphere);
            body = sphere;
        }
        ~DartPhysicsBody()
        {
            if (body)
            {
                body->release();
                body = nullptr;
            }
        }
        DartPhysicsBody& operator=(DartPhysicsBody& other) = delete;
        DartPhysicsBody(DartPhysicsBody& other) = delete;
        DartPhysicsBody(DartPhysicsBody&& other)
        {
            body = other.body;
            other.body = nullptr;
        }
        DartPhysicsBody& operator=(DartPhysicsBody&& other)
        {
            body = other.body;
            other.body = nullptr;
            return *this;
        }
    };

    static std::unordered_map<TwoIntTup, std::vector<entt::entity>, TwoIntTupHash> dartsatspots;



    static std::unordered_map<entt::entity, DartPhysicsBody> physicsbodies;



    struct LootDisplayInstance
    {
        glm::vec3 pos;
        glm::vec4 quat;
        glm::vec2 uvoffset;
    };



    static std::vector<LootDisplayInstance> lootDisplayInstances;


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
        "dart1GltfInstanceShader"

    );



    static const jl::ModelAndTextures basemodeltex = jl::loadModel<true>("resources/models/dart1.glb", false);


    glBindVertexArray(basemodeltex.modelGLObjects.at(0).vao);

    glUseProgram(shader.shaderID);

    //glUniformMatrix4fv(glGetUniformLocation(gltfShader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, basemodeltex.texids.at(0));


    glUniform1i(glGetUniformLocation(shader.shaderID, "texture1"), 0);

    const auto & camera = scene->our<jl::Camera>();

    glUniform1f(glGetUniformLocation(shader.shaderID, "hideClose"), 0.0f);
    glUniform3f(glGetUniformLocation(shader.shaderID, "camPos"),0.f, 0.f, 0.f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
    glUniform1f(glGetUniformLocation(shader.shaderID, "time"), glfwGetTime());
    const auto view = reg.view<Dart1, NPPositionComponent>();
    lootDisplayInstances.clear();
    for (auto entity : view)
    {
        //std::cout << "entity: "  << (int)entity << std::endl;
        auto & pos = view.get<NPPositionComponent>(entity);
        glm::vec4 quat(0.0f, 0.0f, 0.0f, 1.0f); // Default identity quaternion

        auto dart = view.get<Dart1>(entity);

        const auto oldblockspot = TwoIntTup(glm::floor(pos.position.x), glm::floor(pos.position.z));

        //Remove old dropsatspots entry if there
        if(dartsatspots.contains(oldblockspot))
        {
            auto & there = dartsatspots.at(oldblockspot);
            auto it = std::find(there.begin(), there.end(), entity);
            if(it != there.end())
            {
                there.erase(it);
            }
            if(there.empty())
            {
                dartsatspots.erase(oldblockspot);
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
            physicsbodies.insert_or_assign(entity, DartPhysicsBody(pos.position, dart.startingDirection));
        }


        //Re-add to dropsatspots
        const auto newblockspot = TwoIntTup(glm::floor(pos.position.x), glm::floor(pos.position.z));
        if(!dartsatspots.contains(newblockspot))
        {
            dartsatspots.insert({newblockspot, std::vector<entt::entity>{ {entity} }});
        } else
        {
            auto & bp = dartsatspots.at(newblockspot);

            if(std::find(bp.begin(), bp.end(), entity) == bp.end())
            {
                bp.push_back(entity);
            }
        }

        physicsbodies.at(entity).collisionCage.updateToSpot(theScene.world, pos.position, deltaTime);
        lootDisplayInstances.emplace_back(pos.position, quat, glm::vec2(0.0f));
    }

    static GLuint instancevbo = 0;

    const auto  &mglo = basemodeltex.modelGLObjects.at(0);

        glBindVertexArray(mglo.vao);

        if (instancevbo == 0 )
        {
            glGenBuffers(1, &instancevbo);
        }

            glBindBuffer(GL_ARRAY_BUFFER, instancevbo);

            glBufferData(GL_ARRAY_BUFFER, sizeof(LootDisplayInstance) * lootDisplayInstances.size(), lootDisplayInstances.data(), GL_DYNAMIC_DRAW);
            // Position attribute (location 2)
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(LootDisplayInstance), (GLvoid*)0);
            glEnableVertexAttribArray(2);
            glVertexAttribDivisor(2, 1);

            // Quaternion attribute (location 3)
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(LootDisplayInstance), (GLvoid*)(3 * sizeof(float)));
            glEnableVertexAttribArray(3);
            glVertexAttribDivisor(3, 1);

            // UVOFFSET of instance (location 5)
            glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(LootDisplayInstance), (GLvoid*)(7 * sizeof(float)));
            glEnableVertexAttribArray(5);
            glVertexAttribDivisor(5, 1);


        glDrawElementsInstanced(mglo.drawmode, mglo.indexcount, mglo.indextype, nullptr, lootDisplayInstances.size());


        std::erase_if(physicsbodies, [&reg](const auto& pair) {
            return !reg.valid(pair.first);
        });


    const auto & campos = scene->our<jl::Camera>().transform.position;
    auto blockspot = TwoIntTup(glm::floor(campos.x), glm::floor(campos.z));
    if(dartsatspots.contains(blockspot))
    {
        auto & bp = dartsatspots.at(blockspot);
        std::erase_if(bp, [&reg](entt::entity ent)
        {
            return !reg.valid(ent);
        });

        for(auto ent : bp)
        {
            auto & dart = reg.get<Dart1>(ent);

                auto thepos = reg.get<NPPositionComponent>(ent).position;

                if(glm::distance(thepos, campos) < 1.5f)
                {
                    scene->particles->particleBurst(thepos,
                                                            10, (MaterialName)JETPACK_PARTICLE_BLOCK, 0.8, 4.0f);
                    pushToMainToNetworkQueue(DealDamage{.playerIndex = scene->myPlayerIndex, .amount = 1.75f, .projectileToDelete = ent, .damager =  ent});
                }


        }
    }
}

