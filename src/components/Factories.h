//
// Created by jack on 5/15/2025.
//

#ifndef FACTORIES_H
#define FACTORIES_H
#include "LootDrop.h"
#include "../PrecompHeader.h"


entt::entity makeLootDrop(entt::registry& registry, LootDrop drop, glm::vec3 position);



#endif //FACTORIES_H
