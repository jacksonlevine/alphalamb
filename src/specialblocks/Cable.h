//
// Created by jack on 4/9/2025.
//

#ifndef CABLE_H
#define CABLE_H


#include "SpecialBlockInfo.h"



inline void setCableBits(World* world, IntTup spot, const glm::vec3& pp )
{
    static std::vector<IntTup> neighbs = {
        IntTup(1, 0, 0),
        IntTup(-1, 0, 0),
        IntTup(0, 0, 1),
        IntTup(0, 0, -1),
        IntTup(0, 1, 0),
        IntTup(0, -1, 0),
    };

    static std::vector<BlockType> bits = {
        CONNECT_X_BIT,
        CONNECT_NEGX_BIT,
        CONNECT_Z_BIT,
        CONNECT_NEGZ_BIT,
        CONNECT_Y_BIT,
        CONNECT_NEGY_BIT
    };

    static std::vector<BlockType> bitsopposite = {
        CONNECT_NEGX_BIT,
        CONNECT_X_BIT,
        CONNECT_NEGZ_BIT,
        CONNECT_Z_BIT,
        CONNECT_NEGY_BIT,
        CONNECT_Y_BIT,
    };
    //std::cout << "Settings cable bits" << std::endl;
    BlockType myBits = CABLE;

    for (int i = 0; i < 6; i++)
    {
        auto here = spot + neighbs[i];
        auto b = world->getRaw(here);
        auto bid = (b & BLOCK_ID_BITS);
        auto mname = static_cast<MaterialName>(bid);
        if (mname == CABLE)
        {
            //std::cout <<"Cable to the " << neighbs[i].x << ", " << neighbs[i].y << ", " << neighbs[i].z << std::endl;
            myBits |= bits[i];
            b |= bitsopposite[i];
            world->set(here, b);
        }
        //also connect to computer
        if (mname == DG_COMPUTERBLOCK)
        {
            myBits |= bits[i];
        }
    }
    world->set(spot, myBits);
}
inline void removeCableBits(World* world, IntTup spot )
{
    static std::vector<IntTup> neighbs = {
        IntTup(1, 0, 0),
        IntTup(-1, 0, 0),
        IntTup(0, 0, 1),
        IntTup(0, 0, -1),
        IntTup(0, 1, 0),
        IntTup(0, -1, 0),
    };

    static std::vector<BlockType> bits = {
        CONNECT_X_BIT,
        CONNECT_NEGX_BIT,
        CONNECT_Z_BIT,
        CONNECT_NEGZ_BIT,
        CONNECT_Y_BIT,
        CONNECT_NEGY_BIT
    };

    static std::vector<BlockType> bitsopposite = {
        CONNECT_NEGX_BIT,
        CONNECT_X_BIT,
        CONNECT_NEGZ_BIT,
        CONNECT_Z_BIT,
        CONNECT_NEGY_BIT,
        CONNECT_Y_BIT,
    };
    for (int i = 0; i < 6; i++)
    {
        auto here = spot + neighbs[i];
        auto b = world->getRaw(here);
        auto bid = (b & BLOCK_ID_BITS);
        auto mname = static_cast<MaterialName>(bid);
        if (mname == CABLE)
        {
            b ^= bitsopposite[i];
            world->set(here, b);
        }
    }
    world->set(spot, AIR);
}

inline void addCable(UsableMesh& mesh, BlockType block, IntTup position, PxU32& index, PxU32& tindex)
{
    constexpr float postDistFromEdge = 0.3f;
    static std::vector<PxVec3> basePost = {
        //Front
        PxVec3(postDistFromEdge, postDistFromEdge, postDistFromEdge),
        PxVec3(postDistFromEdge, 1.0f - postDistFromEdge, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, postDistFromEdge),

        //Right
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, 1.0f - postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, 1.0f - postDistFromEdge),

        //Back
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, 1.0f - postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, 1.0f - postDistFromEdge, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, postDistFromEdge, 1.0f - postDistFromEdge),

        //Left
        PxVec3(postDistFromEdge, postDistFromEdge, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, 1.0f - postDistFromEdge, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, 1.0f - postDistFromEdge, postDistFromEdge),
        PxVec3(postDistFromEdge, postDistFromEdge, postDistFromEdge),

        //Top
        PxVec3(postDistFromEdge, 1.0f - postDistFromEdge, 1.0f - postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, 1.0f - postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, postDistFromEdge),
        PxVec3(postDistFromEdge, 1.0f - postDistFromEdge, postDistFromEdge),

        //Bottom
        PxVec3(postDistFromEdge, postDistFromEdge, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, postDistFromEdge, 1.0f - postDistFromEdge),
    };

    static std::vector<ColorPack> basePostBrightnesses = {

        ColorPack((uint8_t)12u),
        ColorPack((uint8_t)9u),
        ColorPack((uint8_t)8u),
        ColorPack((uint8_t)7u),
        ColorPack((uint8_t)15u),
        ColorPack((uint8_t)6u),

    };

    static std::vector<ColorPack> baseConnectyBitBrightnesses = {

        ColorPack((uint8_t)8u),
        ColorPack((uint8_t)15u),
        ColorPack((uint8_t)10u),
        ColorPack((uint8_t)6u),
    };

    static std::vector<PxVec3> baseConnectyBit = { // Corrected clockwise ordering for each face
        // Front (clockwise, viewed from outside)
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, postDistFromEdge ),   // Bottom-left
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, postDistFromEdge ), // Top-left
        PxVec3(1.0f, 1.0f - postDistFromEdge, postDistFromEdge ),               // Top-right
        PxVec3(1.0f, postDistFromEdge, postDistFromEdge ),                     // Bottom-right

        // Top (clockwise, viewed from outside)
        PxVec3(1.0f, 1.0f - postDistFromEdge, postDistFromEdge ),              // Bottom-left
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, postDistFromEdge ), // Top-left
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, (1.0f - postDistFromEdge) ), // Top-right
        PxVec3(1.0f, 1.0f - postDistFromEdge, (1.0f - postDistFromEdge) ),     // Bottom-right

        // Back (clockwise, viewed from outside)
        PxVec3(1.0f - postDistFromEdge, 1.0f - postDistFromEdge, (1.0f - postDistFromEdge) ), // Top-left
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, (1.0f - postDistFromEdge) ), // Bottom-left
        PxVec3(1.0f, postDistFromEdge, (1.0f - postDistFromEdge) ),                    // Bottom-right
        PxVec3(1.0f, 1.0f - postDistFromEdge, (1.0f - postDistFromEdge) ),             // Top-right


        // Bottom (clockwise, viewed from outside)
        PxVec3(1.0f, postDistFromEdge, (1.0f - postDistFromEdge) ),                  // Bottom-left
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, (1.0f - postDistFromEdge) ), // Top-left
        PxVec3(1.0f - postDistFromEdge, postDistFromEdge, postDistFromEdge ),          // Top-right
        PxVec3(1.0f, postDistFromEdge, postDistFromEdge ),                            // Bottom-right
    };

    static std::vector<BlockType> bits = {
    CONNECT_X_BIT,
    CONNECT_Z_BIT,
    CONNECT_NEGX_BIT,
    CONNECT_NEGZ_BIT,
    CONNECT_Y_BIT,
    CONNECT_NEGY_BIT};

    static std::vector<std::vector<PxVec3>> connectyBits = {
        baseConnectyBit,
        rotateCoordinatesAroundYNegative90(baseConnectyBit, 1),
        rotateCoordinatesAroundYNegative90(baseConnectyBit, 2),
        rotateCoordinatesAroundYNegative90(baseConnectyBit, 3),
        rotateCoordinatesAroundZNegative90(baseConnectyBit, 3),
        rotateCoordinatesAroundZNegative90(baseConnectyBit, 1),
    };

    for (int i = 0; i < 6; i++)
    {
        if ((block & bits[i]))
        {
            addShapeWithMaterial(connectyBits[i], baseConnectyBitBrightnesses, CABLE, mesh, position, index, tindex);
        }
    }


    addShapeWithMaterial(basePost, basePostBrightnesses, CABLE, mesh, position, index, tindex);



}



#endif //CABLE_H
