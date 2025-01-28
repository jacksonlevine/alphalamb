//
// Created by jack on 1/27/2025.
//

#ifndef HASHMAPUSERDATAMAP_H
#define HASHMAPUSERDATAMAP_H
#include "../UserDataMap.h"

///A simple implementation of UserDataMap using an unordered_map
class HashMapUserDataMap : public UserDataMap{
    std::optional<uint32_t> get(IntTup spot) const override;
private:
    std::unordered_map<IntTup, uint32_t, IntTupHash> map;
};



#endif //HASHMAPUSERDATAMAP_H
