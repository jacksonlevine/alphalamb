//
// Created by jack on 1/27/2025.
//

#include "World.h"


uint32_t World::get(IntTup spot)
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

uint32_t World::getLocked(IntTup spot)
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
