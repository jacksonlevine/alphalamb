//
// Created by jack on 5/15/2025.
//

#include "Factories.h"

#include "Dart1.h"
#include "Lifetime.h"
#include "LootDrop.h"
#include "NPPositionComponent.h"
#include "Orange1.h"
#include "StormyCloud.h"
#include "../EntityWithNameAdder.h"


entt::entity makeLootDrop(entt::registry& registry, LootDrop drop, glm::vec3 position, entt::entity useThisName)
{
    entt::entity loot = addEntityWithEnforcedName(registry, useThisName);


    registry.emplace<NPPositionComponent>(loot, position);
    registry.emplace<Lifetime>(loot, (uint8_t)254);
    registry.emplace<LootDrop>(loot, drop);
    return loot;
}



entt::entity makeStormyCloud(entt::registry& reg, glm::vec3 position, entt::entity useThisName)
{
    entt::entity cloud = addEntityWithEnforcedName(reg, useThisName);

    reg.emplace<NPPositionComponent>(cloud, position);
    reg.emplace<StormyCloud>(cloud);
    return cloud;
}

entt::entity makeOrange1Guy(entt::registry& reg, glm::vec3 position, entt::entity useThisName)
{
    entt::entity guy = addEntityWithEnforcedName(reg, useThisName);
    reg.emplace<NPPositionComponent>(guy, position);
    reg.emplace<Orange1>(guy);

    return guy;
}

entt::entity makeDart1(entt::registry& reg, glm::vec3 position, glm::vec3 direction, float damage, entt::entity useThisName)
{
    entt::entity dart = addEntityWithEnforcedName(reg, useThisName);
    reg.emplace<NPPositionComponent>(dart, position);
    reg.emplace<Lifetime>(dart, (uint8_t)4);
    reg.emplace<Dart1>(dart, direction, damage);

    return dart;
}