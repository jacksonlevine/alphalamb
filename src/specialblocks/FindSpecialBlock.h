//
// Created by jack on 3/11/2025.
//

#ifndef FINDSPECIALBLOCK_H
#define FINDSPECIALBLOCK_H

#include "Cable.h"
#include "Fence.h"
#include "SpecialBlockInfo.h"
#include "Stairs.h"
#include "../PrecompHeader.h"
#include "Computer.h"
#include "Door.h"
#include "Grassy.h"
constexpr std::optional<addBlockFunc> findSpecialBlockMeshFunc(MaterialName bt)
{
    switch (bt)
    {
    case FENCE:
        return addFence;
    case DOOR:
        return addDoor;
    case STONE_STAIRS:
        return addStairs<STONE>;
    case WOOD_STAIRS:
        return addStairs<WOOD_PLANKS>;
    case DG_COMPUTERBLOCK:
        return addComputer;
    case CABLE:
        return addCable;
    case TALL_GRASS:
        return addGrassy;
    default:
        return std::nullopt;
    }
}

constexpr std::optional<setBitsFunc> findSpecialSetBits(MaterialName bt)
{
    switch (bt)
    {
        case FENCE:
            return setFenceBits;
        case DOOR:
            return setDoorBits;
        case STONE_STAIRS:
            return setStairBits<STONE_STAIRS>;
        case WOOD_STAIRS:
            return setStairBits<WOOD_STAIRS>;
        case DG_COMPUTERBLOCK:
            return setComputerBits;
        case CABLE:
            return setCableBits;
        default:
            return std::nullopt;
    }
}

constexpr std::optional<removeBitsFunc> findSpecialRemoveBits(MaterialName bt)
{
    switch (bt)
    {
    case DOOR:
        return removeDoorBits;
    case FENCE:
        return removeFenceBits;
    case CABLE:
        return removeCableBits;
    default:
        return std::nullopt;
    }
}



#endif //FINDSPECIALBLOCK_H
