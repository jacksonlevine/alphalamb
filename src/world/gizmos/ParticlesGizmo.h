//
// Created by jack on 2/3/2025.
//

#ifndef PARTICLESGIZMO_H
#define PARTICLESGIZMO_H

#include "../WorldGizmo.h"
#include "../../PhysXStuff.h"


struct ParticleInstance
{
    glm::vec3 position = glm::vec3(0, 0, 0);
    float scale = 1.0f;
    float blockID = 1.0f;
    PxRigidDynamic* body = nullptr;
    float timeExisted = 0.0f;

    ParticleInstance(glm::vec3 position, float scale, float blockID);
    ParticleInstance() = default;
    ParticleInstance(const ParticleInstance& other) = delete;
    ParticleInstance(ParticleInstance&& other)
    {
        if (this == &other) return;
        position = other.position;
        scale = other.scale;
        blockID = other.blockID;
        timeExisted = other.timeExisted;
        body = other.body;
        other.body = nullptr;
    }
    ParticleInstance& operator=(const ParticleInstance& other) = delete;
    ParticleInstance& operator=(ParticleInstance&& other) noexcept
    {
        if (this == &other) return *this;

        position = other.position;
        scale = other.scale;
        blockID = other.blockID;
        timeExisted = other.timeExisted;
        body = other.body;
        other.body = nullptr;
        return *this;
    }

    ~ParticleInstance()
    {
        if (body == nullptr) return;
        //Detach shape from the actor so it doesnt release our shape too
        PxU32 numShapes = body->getNbShapes();
        std::vector<PxShape*> shapes(numShapes);
        body->getShapes(shapes.data(), numShapes);
        for (PxShape* shape : shapes) {
            body->detachShape(*shape);
        }

        // Release the actor
        body->release();
        body = nullptr;

    }
};


class ParticlesGizmo : public WorldGizmo {
public:
    void draw(World* world, entt::entity playerIndex, entt::registry& reg) override;
    void init() override;

    void addParticle(glm::vec3 position, BlockType blockID, float scale, PxVec3 velocity)
    {




        ParticleInstance instance(position, scale, (float)blockID);


        instances.push_back(std::move(instance));


        instances.back().body->setLinearVelocity(velocity, true);
        //instances.back().body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
        //instances.back().body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);



    }

    void cleanUpOldParticles(float deltaTime)
    {
        for (auto& instance : instances)
        {
            instance.timeExisted += deltaTime;
        }
        auto it = std::remove_if(instances.begin(), instances.end(), [](ParticleInstance& instance) {
            if (instance.timeExisted > 2.0f) {
                return true;
            }
            return false;
        });
        instances.erase(it, instances.end());
    }


    void particleBurst(glm::vec3 spot, size_t amount, BlockType blockID, float width, float energy)
    {
        static FastNoiseLite* noise = new FastNoiseLite();
        static auto _ = [] {
            noise->SetNoiseType(FastNoiseLite::NoiseType_Value);
            noise->SetSeed(time(NULL));
            return 0;
        }();

        for(int i = 0; i < amount; i++)
        {
            glm::vec3 here(
                noise->GetNoise(26.0f*(i) * energy, 26.0f*(i) * energy) * width,
                noise->GetNoise(26.0f*(i) * energy, 26.0f*(i) * energy) * width,
                noise->GetNoise(26.0f*(i) * energy, 26.0f*(i) * energy) * width
                );
            //std::cout << here.x << " " << here.y << " " << here.z << std::endl;
            addParticle(
                here + spot,
                blockID,
                0.3f,
                PxVec3(here.x, here.y, here.z)
                );
        }
    }

    void updateParticlesToPhysicsSpots()
    {
        for (auto& instance : instances)
        {
            if (instance.body)
            {
                    auto trans = instance.body->getGlobalPose();
                    auto pos = trans.p;
                    //std::cout << "Position: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
                    instance.position = glm::vec3(pos.x, pos.y, pos.z);
            }
        }
    }

    size_t maxParticles = 10000;
    bool bufferInitialized = false;

    void sendUpdatedInstancesList() {
        if (instances.empty()) return;

        glBindBuffer(GL_ARRAY_BUFFER, instancesvbo);

        if (!bufferInitialized) {
            glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(ParticleInstance), nullptr, GL_DYNAMIC_DRAW);
            bufferInitialized = true;

            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)0);
            glEnableVertexAttribArray(3);
            glVertexAttribDivisor(3, 1);

            glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)sizeof(glm::vec4));
            glEnableVertexAttribArray(2);
            glVertexAttribDivisor(2, 1);
        }

        if (instances.size() > maxParticles)
        {
            glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(ParticleInstance), instances.data(), GL_DYNAMIC_DRAW);
        } else [[likely]]
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0, instances.size() * sizeof(ParticleInstance), instances.data());
        }

    }
private:
    GLuint vao = 0;
    GLuint mainvbo = 0;

    GLuint instancesvbo = 0;
    GLuint shaderProgram = 0;

    PxRigidStatic* particleCollCage = nullptr;


#ifdef DEBUGDRAW
    ChunkGLInfo cgl = {};
#endif


    std::vector<ParticleInstance> instances = {};

    void updateParticlesCollisionCage(World* world)
    {
        UsableMesh mesh = {};

        PxU32 index = 0;
        PxU32 tindex = 0;

        auto lock = world->tryToGetReadLockOnDMsOnly();
        if (lock != std::nullopt)
        {
            for (auto & particle : instances)
            {
                IntTup spot = IntTup(std::floor(particle.position.x), std::floor(particle.position.y + 0.5f), std::floor(particle.position.z));
                while (world->getLocked(spot) == AIR && spot.y > 0)
                {
                    spot.y -= 1;
                }
                auto wl = (MaterialName)(world->getLocked(spot));
                if (wl != AIR && noColl.test(wl))
                {
                    addFace(PxVec3(spot.x, spot.y, spot.z), Side::Top, GRASS, 1, mesh, index, tindex);
                }

            }

            if(particleCollCage == nullptr)
            {
                particleCollCage = createStaticMeshCollider(PxVec3(0,0,0), mesh.positions, mesh.indices);
            } else
            {
                editStaticMeshCollider(particleCollCage, PxVec3(0,0,0), mesh.positions, mesh.indices);
            }

               #ifdef DEBUGDRAW
                    modifyOrInitializeDrawInstructions(cgl.vvbo, cgl.uvvbo, cgl.ebo, cgl.drawInstructions, mesh, cgl.bvbo, cgl.tvvbo, cgl.tuvvbo, cgl.tebo, cgl.tbvbo);
                #endif


                #ifdef DEBUGDRAW
                if(cgl.drawInstructions.vao != 0)
                {
                    drawFromDrawInstructions(cgl.drawInstructions);
                }
                #endif
        }

    }
};



void sendBlockSideTexturesToShader(GLuint shaderProgram);


#endif //PARTICLESGIZMO_H
