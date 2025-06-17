//
// Created by jack on 5/15/2025.
//

#ifndef FACTORIES_H
#define FACTORIES_H
#include "LootDrop.h"
#include "../PrecompHeader.h"


entt::entity makeLootDrop(entt::registry& registry, LootDrop drop, glm::vec3 position, entt::entity useThisName = entt::null);
entt::entity makeStormyCloud(entt::registry& reg, glm::vec3 position, entt::entity useThisName = entt::null);

entt::entity makeOrange1Guy(entt::registry& reg, glm::vec3 position, entt::entity useThisName);
entt::entity makeDart1(entt::registry& reg, glm::vec3 position, glm::vec3 direction, float damage, entt::entity useThisName);
#endif //FACTORIES_H
