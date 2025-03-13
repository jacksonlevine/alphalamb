//
// Created by jack on 3/11/2025.
//

#ifndef FINDSPECIALBLOCK_H
#define FINDSPECIALBLOCK_H

#include "Fence.h"
#include "SpecialBlockInfo.h"
#include "Stairs.h"
#include "../PrecompHeader.h"

constexpr std::optional<addBlockFunc> findSpecialBlockMeshFunc(MaterialName bt)
{
    switch (bt)
    {
    case FENCE:
        return addFence;
    case STONE_STAIRS:
        return addStairs<STONE>;
    case WOOD_STAIRS:
        return addStairs<WOOD_PLANKS>;
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
        case STONE_STAIRS:
            return setStairBits<STONE_STAIRS>;
        case WOOD_STAIRS:
            return setStairBits<WOOD_STAIRS>;
        default:
            return std::nullopt;
    }
}

#endif //FINDSPECIALBLOCK_H
