//
// Created by jack on 1/27/2025.
//


#ifndef HASHMAPUSERDATAMAP_H
#define HASHMAPUSERDATAMAP_H

#include "../DataMap.h"

//#define MEASURE_LOOKUP 1

///A simple implementation of UserDataMap using an unordered_map (And with optionally enabled measuring of lookup time)
class HashMapDataMap : public DataMap {
public:
    std::optional<uint32_t> get(const IntTup& spot) const override;
    void set(const IntTup& spot, uint32_t block) override;
private:
    std::unordered_map<IntTup, uint32_t, IntTupHash> map;
    mutable std::shared_mutex mutex = {};
#ifdef MEASURE_LOOKUP
    mutable size_t lookup_count = 0;
    mutable std::chrono::duration<double> cumulative_lookup_time = std::chrono::duration<double>::zero();
#endif
};

#endif // HASHMAPUSERDATAMAP_H