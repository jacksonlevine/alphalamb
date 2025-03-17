//
// Created by jack on 1/28/2025.
//

#ifndef WORLDGIZMO_H
#define WORLDGIZMO_H
#include "World.h"
#include "../Player.h"

//Inferface for an interaction gizmo that renders on the world.

class WorldGizmo
{
public:
    virtual void draw(World* world, entt::entity playerIndex, entt::registry& reg) = 0;
    virtual void init() = 0;
};

#endif //WORLDGIZMO_H
