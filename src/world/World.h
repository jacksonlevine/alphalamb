//
// Created by jack on 1/27/2025.
//

#ifndef WORLD_H
#define WORLD_H
#include "UserDataMap.h"
#include "WorldGenMethod.h"


class World {
public:
    World::World(UserDataMap* udm, WorldGenMethod* wm) :
    userDataMap(udm), worldGenMethod(wm) {};

    UserDataMap* userDataMap;
    WorldGenMethod* worldGenMethod;
};



#endif //WORLD_H
