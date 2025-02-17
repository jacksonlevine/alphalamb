//
// Created by jack on 1/27/2025.
//

#include "World.h"


uint32_t World::get(const IntTup& spot)
{
    return (getRaw(spot) & BLOCK_ID_BITS);
}

uint32_t World::getLocked(IntTup spot)
{
    return (getRawLocked(spot) & BLOCK_ID_BITS);
}

uint32_t World::getRaw(IntTup spot)
{
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

uint32_t World::getRawLocked(IntTup spot)
{
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

void World::set(IntTup spot, uint32_t val)
{
    userDataMap->set(spot, val);
}
