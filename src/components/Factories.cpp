//
// Created by jack on 5/15/2025.
//

#include "Factories.h"

#include "Lifetime.h"
#include "LootDrop.h"
#include "NPPositionComponent.h"
#include "StormyCloud.h"
#include "../EntityWithNameAdder.h"


entt::entity makeLootDrop(entt::registry& registry, LootDrop drop, glm::vec3 position, entt::entity useThisName)
{
    entt::entity loot = addEntityWithEnforcedName(registry, useThisName);

    registry.emplace<LootDrop>(loot, drop);
    registry.emplace<NPPositionComponent>(loot, position);
    registry.emplace<Lifetime>(loot, (uint8_t)5);
    return loot;
}



entt::entity makeStormyCloud(entt::registry& reg, glm::vec3 position, entt::entity useThisName)
{
    entt::entity cloud = addEntityWithEnforcedName(reg, useThisName);
    reg.emplace<StormyCloud>(cloud);
    reg.emplace<NPPositionComponent>(cloud, position);
    return cloud;
}