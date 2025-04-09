//
// Created by jack on 3/11/2025.
//

#ifndef FENCE_H
#define FENCE_H

#include "SpecialBlockInfo.h"




inline void setFenceBits(World* world, IntTup spot, const glm::vec3& pp )
{
    static std::vector<IntTup> neighbs = {
        IntTup(1, 0, 0),
        IntTup(-1, 0, 0),
        IntTup(0, 0, 1),
        IntTup(0, 0, -1),
    };

    static std::vector<BlockType> bits = {
        CONNECT_X_BIT,
        CONNECT_NEGX_BIT,
        CONNECT_Z_BIT,
        CONNECT_NEGZ_BIT,
    };

    static std::vector<BlockType> bitsopposite = {
        CONNECT_NEGX_BIT,
        CONNECT_X_BIT,
        CONNECT_NEGZ_BIT,
        CONNECT_Z_BIT,
    };
    //std::cout << "Settings fence bits" << std::endl;
    BlockType myBits = FENCE;

    for (int i = 0; i < 4; i++)
    {
        auto here = spot + neighbs[i];
        auto b = world->getRaw(here);
        auto bid = (b & BLOCK_ID_BITS);
        auto mname = static_cast<MaterialName>(bid);
        if (mname == FENCE)
        {
            //std::cout <<"Fence to the " << neighbs[i].x << ", " << neighbs[i].y << ", " << neighbs[i].z << std::endl;
            myBits |= bits[i];
            b |= bitsopposite[i];
            world->set(here, b);
        }
    }
    world->set(spot, myBits);
}

inline void addFence(UsableMesh& mesh, BlockType block, IntTup position, PxU32& index, PxU32& tindex)
{
    constexpr float postDistFromEdge = 0.3f;
    static std::vector<PxVec3> basePost = {
        //Front
        PxVec3(postDistFromEdge, 0.0f, postDistFromEdge),
        PxVec3(postDistFromEdge, 1.0f, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 1.0f, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 0.0f, postDistFromEdge),

        //Right
        PxVec3(1.0f - postDistFromEdge, 0.0f, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 1.0f, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 1.0f, 1.0f - postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 0.0f, 1.0f - postDistFromEdge),

        //Back
        PxVec3(1.0f - postDistFromEdge, 0.0f, 1.0f - postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 1.0f, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, 1.0f, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, 0.0f, 1.0f - postDistFromEdge),

        //Left
        PxVec3(postDistFromEdge, 0.0f, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, 1.0f, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, 1.0f, postDistFromEdge),
        PxVec3(postDistFromEdge, 0.0f, postDistFromEdge),

        //Top
        PxVec3(postDistFromEdge, 1.0f, 1.0f - postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 1.0f, 1.0f - postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 1.0f, postDistFromEdge),
        PxVec3(postDistFromEdge, 1.0f, postDistFromEdge),

        //Bottom
        PxVec3(postDistFromEdge, 0.0f, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 0.0f, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 0.0f, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, 0.0f, 1.0f - postDistFromEdge),
    };

    static std::vector<float> basePostBrightnesses = {
        0.9f,
        0.8f,
        0.7f,
        0.5f,
        1.0f,
        0.4f
    };

    static std::vector<float> baseConnectyBitBrightnesses = {
        0.6f,
        1.0f,
        0.7f,
        0.4f
    };

    static std::vector<PxVec3> baseConnectyBit = { // Corrected clockwise ordering for each face
        // Front (clockwise, viewed from outside)
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, postDistFromEdge + 0.1f),   // Bottom-left
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, postDistFromEdge + 0.1f), // Top-left
        PxVec3(1.0f, 1.0f - postDistFromEdge, postDistFromEdge + 0.1f),               // Top-right
        PxVec3(1.0f, postDistFromEdge, postDistFromEdge + 0.1f),                     // Bottom-right

        // Top (clockwise, viewed from outside)
        PxVec3(1.0f, 1.0f - postDistFromEdge, postDistFromEdge + 0.1f),              // Bottom-left
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, postDistFromEdge + 0.1f), // Top-left
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, (1.0f - postDistFromEdge) - 0.1f), // Top-right
        PxVec3(1.0f, 1.0f - postDistFromEdge, (1.0f - postDistFromEdge) - 0.1f),     // Bottom-right

        // Back (clockwise, viewed from outside)
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, (1.0f - postDistFromEdge) - 0.1f), // Top-left
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, (1.0f - postDistFromEdge) - 0.1f), // Bottom-left
        PxVec3(1.0f, postDistFromEdge, (1.0f - postDistFromEdge) - 0.1f),                    // Bottom-right
        PxVec3(1.0f, 1.0f - postDistFromEdge, (1.0f - postDistFromEdge) - 0.1f),             // Top-right


        // Bottom (clockwise, viewed from outside)
        PxVec3(1.0f, postDistFromEdge, (1.0f - postDistFromEdge) - 0.1f),                  // Bottom-left
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, (1.0f - postDistFromEdge) - 0.1f), // Top-left
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, postDistFromEdge + 0.1f),          // Top-right
        PxVec3(1.0f, postDistFromEdge, postDistFromEdge + 0.1f),                            // Bottom-right
    };

    static std::vector<BlockType> bits = {
    CONNECT_X_BIT,
    CONNECT_Z_BIT,
    CONNECT_NEGX_BIT,
    CONNECT_NEGZ_BIT,};

    static std::vector<std::vector<PxVec3>> connectyBits = {
        baseConnectyBit,
        rotateCoordinatesAroundYNegative90(baseConnectyBit, 1),
        rotateCoordinatesAroundYNegative90(baseConnectyBit, 2),
        rotateCoordinatesAroundYNegative90(baseConnectyBit, 3),
    };

    for (int i = 0; i < 4; i++)
    {
        if ((block & bits[i]))
        {
            addShapeWithMaterial(connectyBits[i], baseConnectyBitBrightnesses, WOOD_PLANKS, mesh, position, index, tindex);
        }
    }


    addShapeWithMaterial(basePost, basePostBrightnesses, WOOD_PLANKS, mesh, position, index, tindex);



}




#endif //FENCE_H
