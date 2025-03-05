//
// Created by jack on 3/5/2025.
//

#ifndef CREATUREWORLD_H
#define CREATUREWORLD_H

#include "../PrecompHeader.h"

struct CreatureWorld
{
     entt::registry registry = {};


     void update()
     {
          auto view = registry.view;
     }
};



#endif //CREATUREWORLD_H
