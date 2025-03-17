//
// Created by jack on 3/16/2025.
//

#ifndef INVENTORYCOMPONENT_H
#define INVENTORYCOMPONENT_H

#include "../Inventory.h"

struct InventoryComponent {
    MaterialName lastHeldBlock = AIR;
    MaterialName currentHeldBlock = WOOD_PLANKS;
    Inventory inventory = {};
};

#endif //INVENTORYCOMPONENT_H
