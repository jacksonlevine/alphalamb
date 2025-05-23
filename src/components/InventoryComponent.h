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
    MaterialName currentHeldInvIndex = AIR;
    Inventory inventory = {};
    bool add(LootDrop lootDrop)
    {
        int index = 0;
        for (auto & slot : inventory.inventory)
        {
            if(!inventory.isEquipSlot(index)) {
                if (slot.block == AIR && slot.count == 0)
                {
                    slot.block = lootDrop.block;
                    slot.count = lootDrop.count;
                    return true;
                }
                if(slot.block == lootDrop.block && slot.count + lootDrop.count < 99)
                {
                    slot.count += lootDrop.count;
                    return true;
                }
            }
            
            index++;
        }
        return false;
    }
    bool full(LootDrop lootDrop)
    {
        int index = 0;
        for (auto & slot : inventory.inventory)
        {
            if(!inventory.isEquipSlot(index)) {
                if (slot.block == AIR && slot.count == 0)
                {
                    return false;
                }
                if(slot.block == lootDrop.block && slot.count + lootDrop.count < 99)
                {
                    return false;
                }
            }
            index++;
        }
        return true;
    }
    template<typename Archive>
    void serialize(Archive& archive)
    {
        archive(lastHeldBlock, currentHeldInvIndex, inventory);
    }
};

#endif //INVENTORYCOMPONENT_H
