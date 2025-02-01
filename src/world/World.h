//
// Created by jack on 1/27/2025.
//

#ifndef WORLD_H
#define WORLD_H
#include "DataMap.h"
#include "WorldGenMethod.h"


class World {
public:
    World(DataMap* udm, WorldGenMethod* wm, DataMap* nudm) :
    userDataMap(udm), worldGenMethod(wm), nonUserDataMap(nudm) {};

    DataMap* userDataMap;
    DataMap* nonUserDataMap;

    WorldGenMethod* worldGenMethod;

    uint32_t get(IntTup spot);
    uint32_t getLocked(IntTup spot);
    inline std::optional<std::pair<std::shared_lock<std::shared_mutex>,
                                 std::shared_lock<std::shared_mutex>>> tryToGetReadLockOnDMs()
    {
        std::shared_lock<std::shared_mutex> lock1(userDataMap->mutex(), std::try_to_lock);
        if (!lock1.owns_lock()) {
            return std::nullopt;
        }

        std::shared_lock<std::shared_mutex> lock2(nonUserDataMap->mutex(), std::try_to_lock);
        if (!lock2.owns_lock()) {
            return std::nullopt;
        }

        return std::make_pair(std::move(lock1), std::move(lock2));
    }
};



#endif //WORLD_H
