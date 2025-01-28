//
// Created by jack on 1/27/2025.
//

#include "CollisionCage.h"

#include "PhysXStuff.h"
#include "world/WorldRenderer.h"



void CollisionCage::updateToSpot(World* world, glm::vec3 spot)
{
    IntTup blockSpot(std::floor(spot.x), std::floor(spot.y), std::floor(spot.z));

    if(blockSpot != lastBlockSpot)
    {
        lastBlockSpot = blockSpot;
        UsableMesh mesh = {};

        PxU32 index = 0;

        for(int x = -2; x < 2; x++)
        {
            for(int z = -2; z < 2; z++)
            {
                for(int y = 3; y > -3; y--)
                {
                    IntTup spotHere = blockSpot + IntTup(x,y,z);
                    if(world->get(spotHere) != AIR)
                    {
                        for (int i = 0; i < 6; i++)
                        {
                            addFace(PxVec3(spotHere.x, spotHere.y, spotHere.z), (Side)i, GRASS, 1, mesh, index);
                        }
                    }
                }
            }
        }

#ifdef DEBUGDRAW
        modifyOrInitializeDrawInstructions(cgl.vvbo, cgl.uvvbo, cgl.ebo, cgl.drawInstructions, mesh, cgl.bvbo);
#endif

        if(collider == nullptr)
        {
            collider = createStaticMeshCollider(PxVec3(0,0,0), mesh.positions, mesh.indices);
        } else
        {
            editStaticMeshCollider(collider, PxVec3(0,0,0), mesh.positions, mesh.indices);
        }
    }
#ifdef DEBUGDRAW
    if(cgl.drawInstructions.vao != 0)
    {
        drawFromDrawInstructions(cgl.drawInstructions);
    }
#endif

}
