//
// Created by jack on 2/3/2025.
//

#ifndef PARTICLESGIZMO_H
#define PARTICLESGIZMO_H

#include "../WorldGizmo.h"
#include "../../PhysXStuff.h"

#define DEBUGDRAW

struct ParticleInstance
{
    glm::vec3 position = glm::vec3(0, 0, 0);
    float scale = 1.0f;
    float blockID = 1.0f;
    PxRigidDynamic* body = nullptr;
    float timeExisted = 0.0f;

    void destroyThis()
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
    }
};


class ParticlesGizmo : public WorldGizmo {
public:
    void draw(World* world, Player* player) override;
    void init() override;

    void addParticle(glm::vec3 position, uint32_t blockID, float scale)
    {
        PxBoxGeometry boxGeometry(0.1f, 0.1f, 0.1f);

        //Static shape and material (could probably just have some global shape and material but we will see we will have them here for now)
        static PxMaterial* gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.1f);
        static PxShape* shape = gPhysics->createShape(boxGeometry, *gMaterial);

        static bool filterdataset = false;

        if (!filterdataset)
        {
            PxFilterData filterData;
            filterData.word0 = 1 << 0;
            filterData.word1 = 1 << 0;
            shape->setSimulationFilterData(filterData);
            filterdataset = true;
        }

        instances.emplace_back(position, scale, (float)blockID, nullptr, 0.0f);
        instances.back().body = PxCreateDynamic(
            *gPhysics,
            PxTransform(PxVec3(position.x, position.y, position.z)),
            *shape,  // Pass the shape here
            4.0f    // Density
        );

        //instances.back().body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
        //instances.back().body->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);

        gScene->addActor(*instances.back().body);

    }

    void cleanUpOldParticles(float deltaTime)
    {
        for (auto& instance : instances)
        {
            instance.timeExisted += deltaTime;
        }
        auto it = std::remove_if(instances.begin(), instances.end(), [](ParticleInstance& instance) {
        if (instance.timeExisted > 2.0f) {
            instance.destroyThis();
            return true;
        }
        return false;
    });
        instances.erase(it, instances.end());
    }


    void particleBurst(glm::vec3 spot, size_t amount, uint32_t blockID, float width)
    {
        static FastNoiseLite noise;
        static auto _ = [] {
            noise.SetNoiseType(FastNoiseLite::NoiseType_Value); // Run once
            return 0;
        }();
        noise.SetSeed(time(NULL));
        for(int i = 0; i < amount; i++)
        {
            glm::vec3 here(
                noise.GetNoise(26.0f*(spot.x+i), 26.0f*(spot.z-i)) * width,
                noise.GetNoise(26.0f*(spot.x-i), 26.0f*(spot.z+i)) * width,
                noise.GetNoise(26.0f*(spot.y+i), 26.0f*(spot.x-i)) * width
                );
            addParticle(
                here + spot,
                blockID,
                0.3f
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

    void sendUpdatedInstancesList()
    {
        if (instances.empty()) return;
        glBindBuffer(GL_ARRAY_BUFFER, instancesvbo);
        glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(ParticleInstance), instances.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)0);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1);

        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)sizeof(glm::vec4));
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2, 1);
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

        auto lock = world->tryToGetReadLockOnDMs();
        if (lock != std::nullopt)
        {
            for (auto & particle : instances)
            {
                IntTup spot = IntTup(std::floor(particle.position.x), std::floor(particle.position.y), std::floor(particle.position.z));
                while (world->getLocked(spot) == AIR && spot.y > 0)
                {
                    spot.y -= 1;
                }
                if (world->getLocked(spot) != AIR)
                {
                    addFace(PxVec3(spot.x, spot.y, spot.z), Top, GRASS, 1, mesh, index, tindex);
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



inline void sendBlockSideTexturesToShader(GLuint shaderProgram) {
    std::vector<glm::vec2> sideTextures;
    sideTextures.reserve(TEXS.size());

    for (const auto& block : TEXS) {
        sideTextures.push_back(glm::vec2(block[0].first, block[0].second));
    }

    GLint uniformLoc = glGetUniformLocation(shaderProgram, "blockSideTextures");

    glUseProgram(shaderProgram);
    glUniform2fv(uniformLoc, sideTextures.size(), glm::value_ptr(sideTextures[0]));
}


#endif //PARTICLESGIZMO_H
