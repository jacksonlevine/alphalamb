//
// Created by jack on 1/27/2025.
//

#include "World.h"

#include "../BlockType.h"


BlockType World::get(const IntTup& spot)
{
    return (getRaw(spot) & BLOCK_ID_BITS);
}

BlockType World::getLocked(IntTup spot)
{
    return (getRawLocked(spot) & BLOCK_ID_BITS);
}

BlockType World::getRaw(IntTup spot)
{
    // if (auto mem = blockMemo->get(spot); mem != std::nullopt)
    // {
    //     return mem.value();
    // }

    auto id = userDataMap->get(spot);
    if (id == std::nullopt)
    {
        auto nid = nonUserDataMap->get(spot);
        if (nid == std::nullopt)
        {
            return worldGenMethod->get(spot);
        } else
        {
            return nid.value();
        }
    }

    return id.value();
}

BlockType World::getRawLocked(IntTup spot)
{
    // if (auto mem = blockMemo->get(spot); mem != std::nullopt)
    // {
    //     return mem.value();
    // }

    auto id = userDataMap->getLocked(spot);
    if (id == std::nullopt)
    {
        auto nid = nonUserDataMap->getLocked(spot);
        if (nid == std::nullopt)
        {
            return worldGenMethod->get(spot);
        } else
        {
            return nid.value();
        }
    }

    return id.value();
}

void World::set(IntTup spot, const BlockType val)
{
    std::cout << "Setting " << spot.x << " " << spot.y << " " << spot.z << " " << val << std::endl;
    userDataMap->set(spot, val);
    //blockMemo->set(spot, val);
}

void World::setNUDM(const IntTup& spot, const BlockType val)
{
    nonUserDataMap->set(spot, val);
    //blockMemo->set(spot, val);
}
void World::setNUDMLocked(const IntTup& spot, const BlockType val)
{
    nonUserDataMap->setLocked(spot, val);
    //blockMemo->set(spot, val);
}
