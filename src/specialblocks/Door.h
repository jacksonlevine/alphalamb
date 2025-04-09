//
// Created by jack on 4/8/2025.
//

#ifndef DOOR_H
#define DOOR_H


#include "SpecialBlockInfo.h"

inline void addDoorEntity(entt::registry& reg, IntTup spot)
{
}

inline void removeDoorEntity(entt::registry& reg, IntTup spot)
{
}


inline void setDoorBits(World* world, IntTup spot, const glm::vec3& pp)
{
    IntTup playerBlockPos = IntTup(glm::floor(pp.x), glm::floor(pp.y), glm::floor(pp.z));
    auto diff = playerBlockPos - spot;

    // Calculate differences in x and z coordinates
    float diffx = static_cast<float>(diff.x);  // Assuming IntTup has x,y,z members
    float diffz = static_cast<float>(diff.z);

    int direction;
    if (std::abs(diffx) > std::abs(diffz)) {
        direction = (diffx > 0.0f) ? 1 : 3;
    } else {
        direction = (diffz > 0.0f) ? 2 : 0;
    }

    BlockType myBits = DG_DOORBLOCK;
    setDirectionBits(myBits, direction);
    world->set(spot, myBits);
}

inline void addDoor(UsableMesh& mesh, BlockType block, IntTup position, PxU32& index, PxU32& tindex)
{

    auto direction = getDirectionBits(block);
    //std::cout << "direction: " << direction << std::endl;

    constexpr float postDistFromEdge = 0.2f;

    // Base half-slab vertices
    static std::vector<float> baseCompModelBrightnesses = {
        0.6f,
        0.5f,
        0.7f,
        0.8f,
        1.0f,
        0.5f,

        0.5f,
        0.4f,
        0.6f,
        0.7f,

        // 0.5f,

        0.6f,
        0.5f,
        0.7f,
        0.8f,
        1.0f,
        0.5f,
    };

    constexpr float baseHeight = 0.2f;
    constexpr float postHeight = 0.3f;
    constexpr float headHeight = 1.0f;
    static std::vector<PxVec3> baseCompModel = {
        //Base part
        // Front
        PxVec3(0.0f, 0.0f, 0.0f),
        PxVec3(0.0f, baseHeight, 0.0f),
        PxVec3(1.0f, baseHeight, 0.0f),
        PxVec3(1.0f, 0.0f, 0.0f),

        // Right
        PxVec3(1.0f, 0.0f, 0.0f),
        PxVec3(1.0f, baseHeight, 0.0f),
        PxVec3(1.0f, baseHeight, 1.0f),
        PxVec3(1.0f, 0.0f, 1.0f),

        // Back
        PxVec3(1.0f, 0.0f, 1.0f),
        PxVec3(1.0f, baseHeight, 1.0f),
        PxVec3(0.0f, baseHeight, 1.0f),
        PxVec3(0.0f, 0.0f, 1.0f),

        // Left
        PxVec3(0.0f, 0.0f, 1.0f),
        PxVec3(0.0f, baseHeight, 1.0f),
        PxVec3(0.0f, baseHeight, 0.0f),
        PxVec3(0.0f, 0.0f, 0.0f),

        // Top
        PxVec3(0.0f, baseHeight, 1.0f),
        PxVec3(1.0f, baseHeight, 1.0f),
        PxVec3(1.0f, baseHeight, 0.0f),
        PxVec3(0.0f, baseHeight, 0.0f),

        // Bottom
        PxVec3(0.0f, 0.0f, 0.0f),
        PxVec3(1.0f, 0.0f, 0.0f),
        PxVec3(1.0f, 0.0f, 1.0f),
        PxVec3(0.0f, 0.0f, 1.0f),



        //Monitor post
        //Front
        PxVec3(postDistFromEdge, baseHeight, postDistFromEdge),
        PxVec3(postDistFromEdge, postHeight, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, postHeight, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, baseHeight, postDistFromEdge),

        //Right
        PxVec3(1.0f - postDistFromEdge, baseHeight, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, postHeight, postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, postHeight, 1.0f - postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, baseHeight, 1.0f - postDistFromEdge),

        //Back
        PxVec3(1.0f - postDistFromEdge, baseHeight, 1.0f - postDistFromEdge),
        PxVec3(1.0f - postDistFromEdge, postHeight, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, postHeight, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, baseHeight, 1.0f - postDistFromEdge),

        //Left
        PxVec3(postDistFromEdge, baseHeight, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, postHeight, 1.0f - postDistFromEdge),
        PxVec3(postDistFromEdge, postHeight, postDistFromEdge),
        PxVec3(postDistFromEdge, baseHeight, postDistFromEdge),


        //Head part

        // // Front
        // PxVec3(0.0f, postHeight, 0.1f),
        // PxVec3(0.0f, headHeight, 0.1f),
        // PxVec3(1.0f, headHeight, 0.1f),
        // PxVec3(1.0f, postHeight, 0.1f),

        // Right
        PxVec3(1.0f, postHeight, 0.0f),
        PxVec3(1.0f, headHeight, 0.0f),
        PxVec3(1.0f, headHeight - 0.2, 1.0f),
        PxVec3(1.0f, postHeight, 1.0f),

        // Back
        PxVec3(1.0f, postHeight, 1.0f),
        PxVec3(1.0f, headHeight - 0.2, 1.0f),
        PxVec3(0.0f, headHeight - 0.2, 1.0f),
        PxVec3(0.0f, postHeight, 1.0f),

        // Left
        PxVec3(0.0f, postHeight, 1.0f),
        PxVec3(0.0f, headHeight - 0.2, 1.0f),
        PxVec3(0.0f, headHeight, 0.0f),
        PxVec3(0.0f, postHeight, 0.0f),

        // Top
        PxVec3(0.0f, headHeight - 0.2, 1.0f),
        PxVec3(1.0f, headHeight - 0.2, 1.0f),
        PxVec3(1.0f, headHeight, 0.0f),
        PxVec3(0.0f, headHeight, 0.0f),

        // Bottom
        PxVec3(0.0f, postHeight, 0.0f),
        PxVec3(1.0f, postHeight, 0.0f),
        PxVec3(1.0f, postHeight, 1.0f),
        PxVec3(0.0f, postHeight, 1.0f),
    };


    static std::vector<std::vector<PxVec3>> dirModels = {
        baseCompModel,
        rotateCoordinatesAroundYNegative90(baseCompModel, 1),
        rotateCoordinatesAroundYNegative90(baseCompModel, 2),
        rotateCoordinatesAroundYNegative90(baseCompModel, 3),
    };


    addShapeWithMaterial(dirModels.at(direction), baseCompModelBrightnesses, DG_DOORBLOCK, mesh, position, index, tindex);



}



#endif //DOOR_H
