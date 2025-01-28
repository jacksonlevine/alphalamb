//
// Created by jack on 1/27/2025.
//

#include "HashMapUserDataMap.h"

std::optional<uint32_t> HashMapUserDataMap::get(IntTup spot) const
{
    std::optional<uint32_t> block = std::nullopt;
    if (map.contains(spot))
    {
        block = map.at(spot);
    }
    return block;
}
