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
          // // To get all entities with Billboard components
          // auto view = registry.view<Billboard>();
          //
          // // Iterate through all entities with Billboard components
          // for (auto entity : view) {
          //      // Access the Billboard component for this entity
          //      Billboard& billboard = view.get<Billboard>(entity);
          //
          //      // Do something with the entity and its Billboard component
          //      // ...
          // }
     }
};



#endif //CREATUREWORLD_H
