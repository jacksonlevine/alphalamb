//
// Created by jack on 1/27/2025.
//

#ifndef WORLD_H
#define WORLD_H
#include "DataMap.h"
#include "WorldGenMethod.h"


class World {
public:
    World(DataMap* udm, WorldGenMethod* wm) :
    userDataMap(udm), worldGenMethod(wm) {};

    DataMap* userDataMap;
    WorldGenMethod* worldGenMethod;

    uint32_t get(IntTup spot);
};



#endif //WORLD_H
