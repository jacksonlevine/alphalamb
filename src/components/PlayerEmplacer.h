//
// Created by jack on 3/16/2025.
//

#ifndef PLAYEREMPLACER_H
#define PLAYEREMPLACER_H

#include "InventoryComponent.h"
#include "MovementComponent.h"
#include "NetworkComponent.h"
#include "ParticleEffectComponent.h"
#include "PhysicsComponent.h"
#include "RenderComponent.h"
#include "UUIDComponent.h"
#include "../PrecompHeader.h"
#include "../Controls.h"

inline void emplacePlayerParts(entt::registry &REG, entt::entity pers, ClientUID id)
{
    REG.emplace<jl::Camera>(pers);
    REG.emplace<RenderComponent>(pers);
    REG.emplace<PhysicsComponent>(pers);
    REG.emplace<Controls>(pers);
    REG.emplace<InventoryComponent>(pers);
    REG.emplace<ParticleEffectComponent>(pers);
    REG.emplace<NetworkComponent>(pers);
    REG.emplace<MovementComponent>(pers);
    REG.emplace<UUIDComponent>(pers, id);
}






#endif //PLAYEREMPLACER_H
