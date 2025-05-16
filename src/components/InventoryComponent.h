//
// Created by jack on 3/16/2025.
//

#ifndef INVENTORYCOMPONENT_H
#define INVENTORYCOMPONENT_H

#include "PrecompHeader.h"
#include "../Inventory.h"
#include "LootDrop.h"

struct InventoryComponent {
    MaterialName lastHeldBlock = AIR;
    MaterialName currentHeldBlock = LIGHT;
    Inventory inventory = {};
    bool add(LootDrop lootDrop)
    {
        for (auto & slot : inventory.inventory)
        {
            if (slot.block == AIR && slot.count == 0)
            {
                slot.block = lootDrop.block;
                slot.count = lootDrop.count;
                return true;
            }
        }
        return false;
    }
    template<typename Archive>
    void serialize(Archive& archive)
    {
        archive(lastHeldBlock, currentHeldBlock, inventory);
    }
};

#endif //INVENTORYCOMPONENT_H
