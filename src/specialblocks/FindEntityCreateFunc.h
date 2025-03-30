//
// Created by jack on 3/30/2025.
//

#ifndef FINDENTITYCREATEFUNC_H
#define FINDENTITYCREATEFUNC_H

#include "Computer.h"
#include "../PrecompHeader.h"

using entityCreateFunc = std::function<void(entt::registry&, IntTup)>;
using entityRemoveFunc = std::function<void(entt::registry&, IntTup)>;

constexpr std::optional<entityCreateFunc> findEntityCreateFunc(MaterialName bt)
{
    switch(bt)
    {
    case DG_COMPUTERBLOCK:
        return addComputerEntity;
    default:
        return std::nullopt;
    }
}

constexpr std::optional<entityRemoveFunc> findEntityRemoveFunc(MaterialName bt)
{
    switch(bt)
    {
    case DG_COMPUTERBLOCK:
        return removeComputerEntity;
    default:
        return std::nullopt;
    }
}

#endif //FINDENTITYCREATEFUNC_H
