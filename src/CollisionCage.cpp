//
// Created by jack on 1/27/2025.
//

#include "CollisionCage.h"

#include "PhysXStuff.h"
#include "specialblocks/FindSpecialBlock.h"
#include "world/WorldRenderer.h"



void CollisionCage::updateToSpot(World* world, glm::vec3 spot, float deltaTime)
{
    IntTup blockSpot(std::floor(spot.x), std::floor(spot.y), std::floor(spot.z));

    if (updateTimer > 0.01f)
    {
        updateTimer = 0.0f;
        if(true)
        {
            UsableMesh mesh = {};

            PxU32 index = 0;
            PxU32 tindex = 0;
            auto lock = world->tryToGetReadLockOnDMs();
            if(lock != std::nullopt)
            {
                for(int x = -6; x < 6; x++)
                {
                    for(int z = -6; z < 6; z++)
                    {
                        for(int y = 6; y > -6; y--)
                        {
                            IntTup spotHere = blockSpot + IntTup(x,y,z);
                            auto rawhere = world->getRawLocked(spotHere);
                            if(rawhere != AIR)
                            {
                                if (auto func = findSpecialBlockMeshFunc((MaterialName)(rawhere & BLOCK_ID_BITS)); func != std::nullopt)
                                {
                                    func.value()(mesh, rawhere, IntTup(spotHere.x, spotHere.y, spotHere.z), index, tindex);
                                } else
                                {
                                    for (int i = 0; i < 6; i++)
                                    {
                                        addFace(PxVec3(spotHere.x, spotHere.y, spotHere.z), (Side)i, GRASS, 1, mesh, index, tindex);
                                    }
                                }

                            }
                        }
                    }
                }

                if(collider == nullptr)
                {
                    collider = createStaticMeshCollider(PxVec3(0,0,0), mesh.positions, mesh.indices);
                } else
                {
                    editStaticMeshCollider(collider, PxVec3(0,0,0), mesh.positions, mesh.indices);
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

CollisionCage::~CollisionCage()
{
    if(collider != nullptr)
    {
        //collider->release();
    }
}
