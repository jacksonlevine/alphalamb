//
// Created by jack on 2/17/2025.
//

#ifndef INVENTORY_H
#define INVENTORY_H
#include "PrecompHeader.h"
#include "world/ItemName.h"
#include "world/MaterialName.h"

struct InventorySlot
{
    uint32_t block = 0;
    int count = 0;
    bool isItem = false;
    bool empty() const
    {
        return count == 0;
    }
};

constexpr int INVWIDTH = 8;
constexpr int INVHEIGHT = 5;



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
    void setSlot(int x, int y, const InventorySlot& slot)
    {
        inventory[y * INVWIDTH + x] = slot;
    };

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

private:
    std::array<InventorySlot, INVWIDTH * INVHEIGHT> inventory = {};
};


inline void imguiInventory(Inventory& inv)
{

    static bool testset = false;
    if (!testset)
    {
        inv.setSlot(3, 2, InventorySlot{
        .block = 5, .count = 10});
        inv.setSlot(4, 2, InventorySlot{
        .block = 1, .count = 1, .isItem = true});
        testset = true;
    }
    for (int j = 0; j < INVHEIGHT; j++)
    {
        for (int i = 0; i < INVWIDTH; i++)
        {
            int index = j*INVWIDTH + i;
            ImGui::PushID(index);
            if (i > 0) ImGui::SameLine();

            auto& slot = inv.getSlot(i, j);
            std::string label = slot.empty() ? std::string("##inv") + std::to_string(index)  :  std::to_string(slot.block) + " " + std::to_string(slot.count);

            bool isEquip = Inventory::isEquipSlot(i,j);
            bool mouseHeldItemEquippable = equippable(static_cast<ItemName>(inv.mouseHeldItem.block));
            if (isEquip)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0,0.3,0.3,0.7));
            }
            if (ImGui::Button(label.c_str(), ImVec2(50,50)))
            {
                bool validswap = true;
                if (isEquip && !mouseHeldItemEquippable)
                {
                    validswap = false;
                }
                if (validswap)
                {
                    auto mouseslot = inv.mouseHeldItem;
                    inv.mouseHeldItem = slot;
                    slot = mouseslot;
                }
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            {
                if (isEquip && !mouseHeldItemEquippable)
                {
                    ImGui::SetTooltip("Cannot equip this item.");
                } else
                if (isEquip && mouseHeldItemEquippable && inv.mouseHeldItem.block != 0)
                {
                    ImGui::SetTooltip("Equip this item.");
                } else if (!slot.empty())
                {
                    if (slot.isItem)
                    {
                        ImGui::SetTooltip(ToString(static_cast<ItemName>(slot.block)));
                    } else
                    {
                        ImGui::SetTooltip(ToString(static_cast<MaterialName>(slot.block)));
                    }

                }

            }
            if (isEquip)
            {
                ImGui::PopStyleColor();
            }
            ImGui::PopID();
        }
        ImGui::NewLine();
    }
}




#endif //INVENTORY_H
