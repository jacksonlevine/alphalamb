//
// Created by jack on 2/24/2025.
//

#ifndef ITEMNAME_H
#define ITEMNAME_H
#include "../BlockType.h"
#include "../PrecompHeader.h"
#include "../EnumToString.h"

DEFINE_ENUM_WITH_STRING_CONVERSIONS(ItemName, BlockType,
    (NONE)
    (JETPACK)
)

constexpr bool equippable(ItemName name)
{
    return name == ItemName::JETPACK || name == ItemName::NONE;
}



#endif //ITEMNAME_H
