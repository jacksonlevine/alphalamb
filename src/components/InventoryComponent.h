//
// Created by jack on 3/16/2025.
//

#ifndef INVENTORYCOMPONENT_H
#define INVENTORYCOMPONENT_H

#include "PrecompHeader.h"
#include "../Inventory.h"

struct InventoryComponent {
    MaterialName lastHeldBlock = AIR;
    MaterialName currentHeldBlock = LIGHT;
    Inventory inventory = {};
    template<typename Archive>
    void serialize(Archive& archive)
    {
        archive(lastHeldBlock, currentHeldBlock, inventory);
    }
};

#endif //INVENTORYCOMPONENT_H
