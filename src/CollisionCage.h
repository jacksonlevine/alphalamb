//
// Created by jack on 1/27/2025.
//
#ifndef THE_COLLISION_CAGE_H
#define THE_COLLISION_CAGE_H

#include "PrecompHeader.h"
#include "world/World.h"
#include "world/WorldRenderer.h"
#include "specialblocks/FindSpecialBlock.h"
#include "PhysXStuff.h"

using namespace physx;

//#define DEBUGDRAW 1

template <uint8_t radius>
class CollisionCage {
public:
    void updateToSpot(World* world, glm::vec3 spot, float deltaTime);
    CollisionCage() = default;
    ~CollisionCage();
    CollisionCage(const CollisionCage&) = delete;
    CollisionCage& operator=(const CollisionCage&) = delete;
    CollisionCage(CollisionCage&& other) noexcept
    {
        this->collider = other.collider;
        this->tcollider = other.tcollider;
        this->lastBlockSpot = other.lastBlockSpot;
        other.collider = nullptr;
        other.tcollider = nullptr;
    }

    CollisionCage& operator=(CollisionCage&& other) noexcept
    {
        if (this != &other) {

            if (this->collider) {
                this->collider->release();
            }
            if (this->tcollider) {
                this->tcollider->release();
            }

            // Move resources
            this->collider = other.collider;
            this->tcollider = other.tcollider;
            this->lastBlockSpot = other.lastBlockSpot;

            // Clear the source
            other.collider = nullptr;
            other.tcollider = nullptr;
        }
        return *this;
    }
    PxRigidStatic* collider = nullptr;
    PxRigidStatic* tcollider = nullptr;
private:
    IntTup lastBlockSpot = IntTup(-9999,9999,-9999);

    float updateTimer = 0.0f;
#ifdef DEBUGDRAW
    ChunkGLInfo cgl = {};
#endif
};



template <uint8_t radius>
void CollisionCage<radius>::updateToSpot(World* world, glm::vec3 spot, float deltaTime)
{
    const IntTup blockSpot(std::floor(spot.x), std::floor(spot.y), std::floor(spot.z));

    if (updateTimer > 0.03f)
    {
        updateTimer = 0.0f;
        if(true)
        {
            UsableMesh mesh = {};

            PxU32 index = 0;
            PxU32 tindex = 0;
            bool locked = false;
            {

                const auto lock = world->tryToGetReadLockOnDMsOnly();
                if (lock != std::nullopt)
                {
                    locked = true;
                    for (int x = -radius; x < radius; x++)
                    {
                        for (int z = -radius; z < radius; z++)
                        {
                            for (int y = radius; y > -radius; y--)
                            {
                                const IntTup offsetHere = IntTup(x, y, z);
                                const IntTup spotHere = blockSpot + offsetHere;

                                if(spotHere == blockSpot) continue;

                                const auto rawhere = world->getRawLocked(spotHere);
                                const auto idhere = (MaterialName)(rawhere & BLOCK_ID_BITS);
                                if (rawhere != AIR && !noColl.test(idhere))
                                {
                                    if (auto func = findSpecialBlockMeshFunc(idhere); func != std::nullopt && !noCustCollShape.test(idhere))
                                    {
                                        func.value()(mesh, rawhere, IntTup(spotHere.x, spotHere.y, spotHere.z), index, tindex);
                                    }
                                    else
                                    {
                                        //const auto normdirtoplayer = glm::normalize(glm::vec3(offsetHere.x, offsetHere.y, offsetHere.z));
                                        // const auto inttupdirtoplayer = IntTup(normdirtoplayer.x, normdirtoplayer.y, normdirtoplayer.z);
                                        // std::cout << "The shitty output from this: " << inttupdirtoplayer.x << " " << inttupdirtoplayer.y << " " << inttupdirtoplayer.z << std::endl;
                                        for (int i = 0; i < 6; i++)
                                        {
                                            addFace(PxVec3(spotHere.x, spotHere.y, spotHere.z), (Side)i, GRASS, 1, mesh, index, tindex);

                                        }



                                    }

                                }

                            }
                        }
                    }
                }
            }
            if (locked) {
                //mesh.positions.insert(mesh.positions.end(), mesh.tpositions.begin(), mesh.tpositions.end());
                //mesh.indices.insert(mesh.indices.end(), mesh.tindices.begin(), mesh.tpositions.end());
                if (collider == nullptr)
                {
                    collider = createStaticMeshCollider(PxVec3(0, 0, 0), mesh.positions, mesh.indices);
                }
                else
                {
                    editStaticMeshCollider(collider, PxVec3(0, 0, 0), mesh.positions, mesh.indices);
                }
                if (tcollider == nullptr)
                {
                    tcollider = createStaticMeshCollider(PxVec3(0, 0, 0), mesh.tpositions, mesh.tindices);
                }
                else
                {
                    editStaticMeshCollider(tcollider, PxVec3(0, 0, 0), mesh.tpositions, mesh.tindices);
                }
            }





#ifdef DEBUGDRAW
            modifyOrInitializeDrawInstructions(cgl.vvbo, cgl.uvvbo, cgl.ebo, cgl.drawInstructions, mesh, cgl.bvbo, cgl.tvvbo, cgl.tuvvbo, cgl.tebo, cgl.tbvbo);
#endif


        }
    } else
    {
        updateTimer += deltaTime;
    }


#ifdef DEBUGDRAW
    if(cgl.drawInstructions.vao != 0)
    {
        drawFromDrawInstructions(cgl.drawInstructions);
    }
#endif

}
template <uint8_t radius>
CollisionCage<radius>::~CollisionCage()
{
    if(tcollider != nullptr)
    {
        collider->release();
        collider = nullptr;
    }
    if(tcollider != nullptr)
    {
        tcollider->release();
        tcollider = nullptr;
    }
}

#endif //THE_COLLISION_CAGE_H