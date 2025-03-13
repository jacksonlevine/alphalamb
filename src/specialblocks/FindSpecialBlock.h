//
// Created by jack on 3/11/2025.
//

#ifndef FINDSPECIALBLOCK_H
#define FINDSPECIALBLOCK_H

#include "Fence.h"
#include "SpecialBlockInfo.h"
#include "Stairs.h"
#include "../PrecompHeader.h"

constexpr std::optional<addBlockFunc> findSpecialBlock(MaterialName bt)
{
    if (bt == FENCE)
    {
        return addFence;
    }
    if (bt == STONE_STAIRS)
    {
        return addStairs;
    }
    return std::nullopt;
}




constexpr std::optional<setBitsFunc> findSpecialSetBits(MaterialName bt)
{
    if (bt == FENCE)
    {
        return setFenceBits;
    }
    if (bt == STONE_STAIRS)
    {
        return setStairBits;
    }
    return std::nullopt;
}

#endif //FINDSPECIALBLOCK_H
