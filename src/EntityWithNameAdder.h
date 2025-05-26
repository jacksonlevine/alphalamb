//
// Created by jack on 5/25/2025.
//

#ifndef ENTITYWITHNAMEADDER_H
#define ENTITYWITHNAMEADDER_H

#include "PrecompHeader.h"

inline entt::entity addEntityWithEnforcedName(entt::registry& reg, entt::entity useThisName)
{
    if (reg.valid(useThisName))
    {
        std::cerr << "Tried to create entity that already was existing. " << std::endl;
        throw std::runtime_error("Tried to create entity that already was existing. Fatal fucking error.");
    }
    entt::entity id;
    if (useThisName == entt::null)
    {
        id = reg.create();
    } else
    {
        id = reg.create(useThisName);
    }
    return id;
}

#endif //ENTITYWITHNAMEADDER_H
