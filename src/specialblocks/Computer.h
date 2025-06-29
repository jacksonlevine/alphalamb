//
// Created by jack on 3/26/2025.
//

#ifndef DGCOMPUTER_H
#define DGCOMPUTER_H

#include "SpecialBlockInfo.h"
#include "../EntityWithNameAdder.h"
#include "../components/ComputerComponent.h"
#include "../components/NPPositionComponent.h"

inline entt::entity addComputerEntity(entt::registry& reg, IntTup spot, entt::entity useThisName = entt::null)
{
    auto id = addEntityWithEnforcedName(reg, useThisName);

    reg.emplace<ComputerComponent>(id);
    reg.emplace<NPPositionComponent>(id, glm::vec3(spot.x, spot.y, spot.z));
    return id;
}

inline void removeComputerEntity(entt::registry& reg, IntTup spot)
{
    auto view = reg.view<ComputerComponent, NPPositionComponent>();

    entt::entity matched = entt::null;
    for(auto entity : view)
    {
        auto & comp = view.get<ComputerComponent>(entity);
        auto & nppos = view.get<NPPositionComponent>(entity);
        if(nppos.position == glm::vec3(spot.x, spot.y, spot.z))
        {
            matched = entity;
            break;
        }
    }
    if(matched != entt::null)
    {
        reg.destroy(matched);
    }
}


inline void setComputerBits(World* world, IntTup spot, const glm::vec3& pp)
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

    BlockType myBits = DG_COMPUTERBLOCK;
    setDirectionBits(myBits, direction);
    world->set(spot, myBits);
}

inline void addComputer(UsableMesh& mesh, BlockType block, IntTup position, PxU32& index, PxU32& tindex)
{

    auto direction = getDirectionBits(block);
    //std::cout << "direction: " << direction << std::endl;

    constexpr float postDistFromEdge = 0.2f;

    // Base half-slab vertices
    static std::vector<ColorPack> baseCompModelBrightnesses = {
        ColorPack((uint8_t)9u),
        ColorPack((uint8_t)8u),
        ColorPack((uint8_t)11u),
        ColorPack((uint8_t)12u),
        ColorPack((uint8_t)15u),
        ColorPack((uint8_t)8u),

        ColorPack((uint8_t)8u),
        ColorPack((uint8_t)6u),
        ColorPack((uint8_t)9u),
        ColorPack((uint8_t)11u),

        ColorPack((uint8_t)9u),
        ColorPack((uint8_t)8u),
        ColorPack((uint8_t)11u),
        ColorPack((uint8_t)12u),
        ColorPack((uint8_t)15u),
        ColorPack((uint8_t)8u),
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


    addShapeWithMaterial(dirModels.at(direction), baseCompModelBrightnesses, DG_COMPUTERBLOCK, mesh, position, index, tindex);



}




#endif //COMPUTER_H
