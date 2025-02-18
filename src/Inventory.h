//
// Created by jack on 2/17/2025.
//

#ifndef INVENTORY_H
#define INVENTORY_H
#include "PrecompHeader.h"

struct InventorySlot
{
    uint32_t block = 0;
    int count = 0;

    bool empty() const
    {
        return count == 0;
    }
};

constexpr int INVWIDTH = 7;
constexpr int INVHEIGHT = 4;

class Inventory {
public:
    std::array<InventorySlot, INVWIDTH * INVHEIGHT> inventory = {};
    InventorySlot mouseHeldItem = {};
    InventorySlot& getSlot(const int x, const int y)
    {
        return inventory[y * INVWIDTH + x];
    }
    void setSlot(int x, int y, const InventorySlot& slot)
    {
        inventory[y * INVWIDTH + x] = slot;
    };
};


inline void imguiInventory(Inventory& inv)
{

    static bool testset = false;
    if (!testset)
    {
        inv.setSlot(3, 2, InventorySlot{
        .block = 5, .count = 10});
        testset = true;
    }
    for (int j = 0; j < INVHEIGHT; j++)
    {
        for (int i = 0; i < INVWIDTH; i++)
        {
            int index = j*INVWIDTH + i;
            ImGui::PushID(index);
            if (i > 0) ImGui::SameLine();

            auto& slot = inv.inventory.at(index);
            std::string label = slot.empty() ? std::string("##inv") + std::to_string(index)  :  std::to_string(slot.block) + " " + std::to_string(slot.count);
            if (ImGui::Button(label.c_str(), ImVec2(50,50)))
            {
                auto mouseslot = inv.mouseHeldItem;
                inv.mouseHeldItem = slot;
                slot = mouseslot;
            }
            ImGui::PopID();
        }
        ImGui::NewLine();
    }
}




#endif //INVENTORY_H
