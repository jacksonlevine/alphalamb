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

        PxU32 index;

        for(int x = -6; x < 6; x++)
        {
            for(int z = -6; z < 6; z++)
            {
                for(int y = 6; y > -6; y--)
                {
                    IntTup spotHere = blockSpot + IntTup(x,y,z);
                    if(world->get(spotHere) != AIR)
                    {
                        for (int i = 0; i < std::size(neighborSpots); i++)
                        {
                            auto neigh = neighborSpots[i];
                            if (world->get(neigh + spotHere) == AIR)
                            {
                                addFace(PxVec3(spotHere.x, spotHere.y, spotHere.z), (Side)i, GRASS, 1, mesh, index);
                            }
                        }
                    }
                }
            }
        }


        modifyOrInitializeDrawInstructions(cgl.vvbo, cgl.uvvbo, cgl.ebo, cgl.drawInstructions, mesh, cgl.bvbo);

        // for(auto & pos : mesh.positions)
        // {
        //     std::cout << pos.x << " " << pos.y << " " << pos.z << " \n";
        // }
        std::cout << "Positions count : " << mesh.positions.size() << " \n";

        if(collider == nullptr)
        {
            collider = createStaticMeshCollider(PxVec3(0,0,0), mesh.positions, mesh.indices);
        } else
        {
            editStaticMeshCollider(collider, PxVec3(0,0,0), mesh.positions, mesh.indices);
        }
    }
    if(cgl.drawInstructions.vao != 0)
    {
        drawFromDrawInstructions(cgl.drawInstructions);
    }

}
