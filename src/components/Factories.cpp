//
// Created by jack on 5/15/2025.
//

#include "Factories.h"

#include "LootDrop.h"
#include "NPPositionComponent.h"
#include "../EntityWithNameAdder.h"


entt::entity makeLootDrop(entt::registry& registry, LootDrop drop, glm::vec3 position, entt::entity useThisName)
{
    entt::entity loot = addEntityWithEnforcedName(registry, useThisName);

    registry.emplace<LootDrop>(loot, drop);
    registry.emplace<NPPositionComponent>(loot, position);
    return loot;
}
