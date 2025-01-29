//
// Created by jack on 1/27/2025.
//


#ifndef HASHMAPUSERDATAMAP_H
#define HASHMAPUSERDATAMAP_H

#include "../UserDataMap.h"

//#define MEASURE_LOOKUP 1

///A simple implementation of UserDataMap using an unordered_map (And with optionally enabled measuring of lookup time)
class HashMapUserDataMap : public UserDataMap {
public:
    std::optional<uint32_t> get(IntTup spot) const override;

private:
    std::unordered_map<IntTup, uint32_t, IntTupHash> map;

#ifdef MEASURE_LOOKUP
    mutable size_t lookup_count = 0;
    mutable std::chrono::duration<double> cumulative_lookup_time = std::chrono::duration<double>::zero();
#endif
};

#endif // HASHMAPUSERDATAMAP_H