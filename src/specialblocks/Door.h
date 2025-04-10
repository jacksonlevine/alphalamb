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

constexpr uint32_t DOOROPEN_BITS = 0b0000'0000'0000'0100'0000'0000'0000'0000;
constexpr uint32_t DOORTOP_BITS = 0b0000'0000'0000'1000'0000'0000'0000'0000;

constexpr uint32_t OPPOSITEDOOR_BITS = 0b0000'0000'0001'0000'0000'0000'0000'0000;


inline uint32_t getOppositeDoorBits(uint32_t input) {
    return (input & OPPOSITEDOOR_BITS) >> 20;
};

inline void setOppositeDoorBits(uint32_t& input, uint32_t bit) {
    uint32_t bits = bit << 20;
    input = input & ~OPPOSITEDOOR_BITS;
    input |= bits;
};

inline uint32_t getDoorOpenBit(uint32_t input) {
    return (input & DOOROPEN_BITS) >> 18;
};

inline void setDoorOpenBit(uint32_t& input, uint32_t open) {

    uint32_t bits = open << 18;
    input = input & ~DOOROPEN_BITS;
    input |= bits;

};

inline void toggleDoorOpenBit(uint32_t& input) {
    input ^= DOOROPEN_BITS;
};

inline uint32_t getDoorTopBit(uint32_t input) {
    return (input & DOORTOP_BITS) >> 19;
};

inline void setDoorBits(World* world, IntTup spot, const glm::vec3& pp)
{
    auto rawWasHere = world->getRaw(spot);
    auto IDwasHere = rawWasHere & BLOCK_ID_BITS;
    if (IDwasHere == DOOR)
    {
        //if its a re-place that means toggle the door
        int top = getDoorTopBit(rawWasHere);
        IntTup otherHalf;
        if(top == 1) {
            otherHalf = spot + IntTup(0, -1, 0);
        } else {
            otherHalf = spot + IntTup(0, 1, 0);
        }
        BlockType rawOtherHalf = world->getRaw(otherHalf);

        toggleDoorOpenBit(rawWasHere);
        toggleDoorOpenBit(rawOtherHalf);

        world->set(spot, rawWasHere);
        world->set(otherHalf, rawOtherHalf);
        return;
    }

    static std::vector<IntTup> neighborAxes = {
        IntTup(1,0,0),
        IntTup(0,0,1),
        IntTup(1,0,0),
        IntTup(0,0,1),
    };

    IntTup placeAbove = spot + IntTup(0,1,0);
    IntTup placeBelow = spot + IntTup(0,-1,0);

    bool condition1 = world->get(placeAbove) == AIR;
    bool condition2 = world->get(placeBelow) != AIR;

    if(condition1 && condition2)
    {
        BlockType bottomBits = DOOR;
        BlockType topBits = DOOR;

        topBits |= DOORTOP_BITS;

        IntTup playerBlockPos = IntTup(glm::floor(pp.x), glm::floor(pp.y), glm::floor(pp.z));
        auto diff = playerBlockPos - spot;

        float diffx = static_cast<float>(diff.x);
        float diffz = static_cast<float>(diff.z);

        int direction;
        if (std::abs(diffx) > std::abs(diffz)) {
            direction = (diffx > 0.0f) ? 1 : 3;
        } else {
            direction = (diffz > 0.0f) ? 2 : 0;
        }

        setDirectionBits(bottomBits, direction);
        setDirectionBits(topBits, direction);

        IntTup left;
        IntTup right;

        if(direction == 0 || direction == 1) {
            left = spot - neighborAxes[direction];
            right = spot + neighborAxes[direction];
        } else {
            left = spot + neighborAxes[direction];
            right = spot - neighborAxes[direction];
        }

        BlockType blockBitsRight = world->getRaw(right);
        BlockType blockBitsLeft = world->getRaw(left);

        if((blockBitsRight & BLOCK_ID_BITS) == DOOR)
        {
            BlockType neighdir = getDirectionBits(blockBitsRight);
            if(neighdir == direction && getDoorTopBit(blockBitsRight) == 0)
            {
                IntTup rightUp = right + IntTup(0,1,0);
                uint32_t neighTopBits = world->getRaw(rightUp);

                setOppositeDoorBits(topBits, 1);
                setOppositeDoorBits(bottomBits, 1);

                setOppositeDoorBits(blockBitsRight, 0);
                setOppositeDoorBits(neighTopBits, 0);

                world->set(right, blockBitsRight);
                world->set(rightUp, neighTopBits);
            }
        }
        if((blockBitsLeft & BLOCK_ID_BITS) == DOOR) {
            //std::cout << "Door to my left! \n";
            uint32_t neighdir = getDirectionBits(blockBitsLeft);
            if(neighdir == direction && getDoorTopBit(blockBitsLeft) == 0) {
                IntTup leftUp = left + IntTup(0,1,0);
                uint32_t neighTopBits = world->getRaw(leftUp);

                setOppositeDoorBits(topBits, 0);
                setOppositeDoorBits(bottomBits, 0);

                setOppositeDoorBits(blockBitsLeft, 1);
                setOppositeDoorBits(neighTopBits, 1);

                world->set(left, blockBitsLeft);
                world->set(leftUp, neighTopBits);

                            
            }
        }
        world->set(spot, bottomBits);
        world->set(placeAbove, topBits);
    }
}

inline void removeDoorBits(World* world, IntTup spot)
{
    uint32_t blockBitsHere = world->getRaw(spot);
    int top = getDoorTopBit(blockBitsHere);
    IntTup otherHalf;
    if(top == 1) {
        otherHalf = spot + IntTup(0, -1, 0);
    } else {
        otherHalf = spot + IntTup(0, 1, 0);
    }
    world->set(otherHalf, AIR);
    world->set(spot, AIR);
}

inline void addDoor(UsableMesh& mesh, BlockType block, IntTup position, PxU32& index, PxU32& tindex)
{

    static std::vector<float> baseDoorModelBrightnesses = {
        0.7f,
        0.7f,
        0.6f,
        0.5f,
        0.3f,
        1.0f
    };

    constexpr float doorthick = 0.15f;
    static std::vector<PxVec3> baseDoorModel = {
        //Front
        PxVec3(0.f, 0.f, 0.f),
        PxVec3(0.f, 1.f, 0.f),
        PxVec3(1.f, 1.f, 0.f),
        PxVec3(1.f, 0.f, 0.f),

        //Back
        PxVec3(1.f, 0.f, doorthick),
        PxVec3(1.f, 1.f, doorthick),
        PxVec3(0.f, 1.f, doorthick),
        PxVec3(0.f, 0.f, doorthick),

        //Right
        PxVec3(1.f, 0.f, 0.f),
        PxVec3(1.f, 1.f, 0.f),
        PxVec3(1.f, 1.f, doorthick),
        PxVec3(1.f, 0.f, doorthick),

        //Left
        PxVec3(0.f, 0.f, doorthick),
        PxVec3(0.f, 1.f, doorthick),
        PxVec3(0.f, 1.f, 0.f),
        PxVec3(0.f, 0.f, 0.f),

        //Bottom
        PxVec3(0.f, 0.f, doorthick),
        PxVec3(0.f, 0.f, 0.f),
        PxVec3(1.f, 0.f, 0.f),
        PxVec3(1.f, 0.f, doorthick),

        //Top
        PxVec3(1.f, 1.f, doorthick),
        PxVec3(1.f, 1.f, 0.f),
        PxVec3(0.f, 1.f, 0.f),
        PxVec3(0.f, 1.f, doorthick),
    };
    static std::vector<std::vector<PxVec3>> dirModels = {
        baseDoorModel,
        rotateCoordinatesAroundYNegative90(baseDoorModel, 1),
        rotateCoordinatesAroundYNegative90(baseDoorModel, 2),
        rotateCoordinatesAroundYNegative90(baseDoorModel, 3),
    };



    int direction = getDirectionBits(block);
    int open = getDoorOpenBit(block);
    int opposite = getOppositeDoorBits(block);

    int modelIndex;
    if(opposite == 1) {
        modelIndex = (direction - open);
        if(modelIndex < 0) {
            modelIndex = 3;
        }
    } else {
        modelIndex = (direction + open) % 4;
    }

    int doorTop = getDoorTopBit(block);

    if (doorTop)
    {
        addShapeWithMaterial(dirModels.at(modelIndex), baseDoorModelBrightnesses, GLASS, mesh, position, index, tindex);

    } else
    {
        addShapeWithMaterial(dirModels.at(modelIndex), baseDoorModelBrightnesses, WOOD_PLANKS, mesh, position, index, tindex);

    }



}



#endif //DOOR_H
