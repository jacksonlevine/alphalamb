//
// Created by jack on 1/27/2025.
//

#include "World.h"


uint32_t World::get(IntTup spot)
{
    auto id = userDataMap->get(spot);
    if (id == std::nullopt)
    {
        return worldGenMethod->get(spot);
    }
    return id.value();
}
