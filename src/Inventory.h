//
// Created by jack on 2/17/2025.
//

#ifndef INVENTORY_H
#define INVENTORY_H


#include "PrecompHeader.h"
#include "world/ItemName.h"
#include "world/MaterialName.h"

constexpr int INVWIDTH = 8;
constexpr int INVHEIGHT = 5;




struct InventorySlot
{
    BlockType block = 0;
    uint8_t count = 0;
    bool isItem = false;
    bool empty() const
    {
        return count == 0;
    }

    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(block, count, isItem);
    }
};




constexpr std::array<InventorySlot, INVWIDTH * INVHEIGHT> DEFAULT_INVENTORY = {
    InventorySlot{ .block = ItemName::JETPACK, .count = 1, .isItem = true},
};

class Inventory {
public:



    InventorySlot mouseHeldItem = {};
    InventorySlot& getSlot(const int x, const int y)
    {
        return inventory[y * INVWIDTH + x];
    }
    static constexpr bool isEquipSlot(int x, int y)
    {
        return x == INVWIDTH-1;
    }

    static constexpr bool isEquipSlot(int index)
    {
        return (index % INVWIDTH) == INVWIDTH-1;
    }
    void setSlot(int x, int y, const InventorySlot& slot)
    {
        inventory[y * INVWIDTH + x] = slot;
    };
    int getIndex(int x, int y)
    {
        return y * INVWIDTH + x;
    }

    std::array<InventorySlot, INVHEIGHT> getEquippedItems()
    {
        std::array<InventorySlot, INVHEIGHT> equippedItems;

        int x = INVWIDTH - 1;
        for (int y = 0; y < INVHEIGHT; y++)
        {
            equippedItems[y] = getSlot(x, y);
        }

        return equippedItems;
    }
    std::array<InventorySlot, INVWIDTH * INVHEIGHT> inventory = DEFAULT_INVENTORY;

    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(mouseHeldItem, inventory);
    }
};



void imguiInventory(Inventory& inv);



#endif //INVENTORY_H
