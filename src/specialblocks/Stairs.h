#ifndef STAIRS_H
#define STAIRS_H

#include "SpecialBlockInfo.h"

template <MaterialName stairID>
void setStairBits(World* world, IntTup spot, const glm::vec3& pp)
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

    // Start with base STAIRS block type
    BlockType myBits = stairID;

    for (int i = 0; i < 4; i++)
    {
        auto here = spot + neighbs[i];
        auto b = world->getRaw(here);
        auto bid = (b & BLOCK_ID_BITS);
        auto mname = static_cast<MaterialName>(bid);

        // Connect to any non-stair block
        if (mname != stairID && mname != AIR)
        {
            myBits |= bits[i];
        }
    }
    world->set(spot, myBits);
}

template <MaterialName stairMaterial>
void addStairs(UsableMesh& mesh, BlockType block, IntTup position, PxU32& index, PxU32& tindex)
{
    // Base half-slab vertices
    static std::vector<ColorPack> baseHalfSlabBrightnesses = {
        ColorPack((uint8_t)9u),
        ColorPack((uint8_t)8u),
        ColorPack((uint8_t)13u),
        ColorPack((uint8_t)14u),
        ColorPack((uint8_t)15u),
        ColorPack((uint8_t)7u),
    };
    static std::vector<PxVec3> baseHalfSlab = {
        // Front
        PxVec3(0.0f, 0.0f, 0.0f),
        PxVec3(0.0f, 0.5f, 0.0f),
        PxVec3(1.0f, 0.5f, 0.0f),
        PxVec3(1.0f, 0.0f, 0.0f),

        // Right
        PxVec3(1.0f, 0.0f, 0.0f),
        PxVec3(1.0f, 0.5f, 0.0f),
        PxVec3(1.0f, 0.5f, 1.0f),
        PxVec3(1.0f, 0.0f, 1.0f),

        // Back
        PxVec3(1.0f, 0.0f, 1.0f),
        PxVec3(1.0f, 0.5f, 1.0f),
        PxVec3(0.0f, 0.5f, 1.0f),
        PxVec3(0.0f, 0.0f, 1.0f),

        // Left
        PxVec3(0.0f, 0.0f, 1.0f),
        PxVec3(0.0f, 0.5f, 1.0f),
        PxVec3(0.0f, 0.5f, 0.0f),
        PxVec3(0.0f, 0.0f, 0.0f),

        // Top
        PxVec3(0.0f, 0.5f, 1.0f),
        PxVec3(1.0f, 0.5f, 1.0f),
        PxVec3(1.0f, 0.5f, 0.0f),
        PxVec3(0.0f, 0.5f, 0.0f),

        // Bottom
        PxVec3(0.0f, 0.0f, 0.0f),
        PxVec3(1.0f, 0.0f, 0.0f),
        PxVec3(1.0f, 0.0f, 1.0f),
        PxVec3(0.0f, 0.0f, 1.0f),
    };
    static std::vector<ColorPack> topBackXBrightnesses = {
        ColorPack((uint8_t)8),
    ColorPack((uint8_t)9),
    ColorPack((uint8_t)7),

    ColorPack((uint8_t)10),

    ColorPack((uint8_t)15),

    };
    // Top back section for +X connection (faces east)
    static std::vector<PxVec3> topBackX = {
        // Front
        PxVec3(0.5f, 0.5f, 0.0f),
        PxVec3(0.5f, 1.0f, 0.0f),
        PxVec3(1.0f, 1.0f, 0.0f),
        PxVec3(1.0f, 0.5f, 0.0f),

        // Right
        PxVec3(1.0f, 0.5f, 0.0f),
        PxVec3(1.0f, 1.0f, 0.0f),
        PxVec3(1.0f, 1.0f, 1.0f),
        PxVec3(1.0f, 0.5f, 1.0f),

        // Back
        PxVec3(1.0f, 0.5f, 1.0f),
        PxVec3(1.0f, 1.0f, 1.0f),
        PxVec3(0.5f, 1.0f, 1.0f),
        PxVec3(0.5f, 0.5f, 1.0f),

        // Left
        PxVec3(0.5f, 0.5f, 1.0f),
        PxVec3(0.5f, 1.0f, 1.0f),
        PxVec3(0.5f, 1.0f, 0.0f),
        PxVec3(0.5f, 0.5f, 0.0f),

        // Top
        PxVec3(0.5f, 1.0f, 1.0f),
        PxVec3(1.0f, 1.0f, 1.0f),
        PxVec3(1.0f, 1.0f, 0.0f),
        PxVec3(0.5f, 1.0f, 0.0f),
    };

    // Define connection bits for each direction
    static std::vector<BlockType> bits = {
        CONNECT_X_BIT,
        CONNECT_NEGX_BIT,
        CONNECT_Z_BIT,
        CONNECT_NEGZ_BIT
    };

    // Create top back sections for all directions by rotating the base +X section
    // Correctly map the rotations for X and -X directions
    static std::vector<std::vector<PxVec3>> topBackSections = {
        topBackX,                                           // +X direction (no rotation)
        rotateCoordinatesAroundYNegative90(topBackX, 2),    // -X direction (rotate 180°)
        rotateCoordinatesAroundYNegative90(topBackX, 1),    // +Z direction (rotate 90°)
        rotateCoordinatesAroundYNegative90(topBackX, 3)     // -Z direction (rotate 270°)
    };

    // First add the base half-slab
    addShapeWithMaterial(baseHalfSlab, baseHalfSlabBrightnesses, stairMaterial, mesh, position, index, tindex);

    // Then add the top back sections for each set connection bit
    for (int i = 0; i < 4; i++)
    {
        if (block & bits[i])
        {
            addShapeWithMaterial(topBackSections[i], topBackXBrightnesses, stairMaterial, mesh, position, index, tindex);
        }
    }
}

#endif // STAIRS_H