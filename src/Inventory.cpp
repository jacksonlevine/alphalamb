//
// Created by jack on 2/17/2025.
//

#include "Inventory.h"

#include "Client.h"
#include "ImGuiStuff.h"

void imguiInventory(Inventory& inv)
{
    // static bool testset = false;
    // if (!testset)
    // {
    //     inv.setSlot(3, 2, InventorySlot{
    //     .block = 5, .count = 10});
    //     inv.setSlot(4, 2, InventorySlot{
    //     .block = 1, .count = 1, .isItem = true});
    //     testset = true;
    // }
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
            //
            // if (isEquip)
            // {
            //     ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0,0.3,0.3,0.7));
            // }

            auto bt = isEquip ? DGButtonType::Bad1 : DGButtonType::Good1;

            if (DGCustomButton(label.c_str(), bt, ImVec2(50,50)))
            {
                bool validswap = true;
                if (isEquip && !mouseHeldItemEquippable)
                {
                    validswap = false;
                }
                if (validswap)
                {
                    pushToMainToNetworkQueue(RequestInventorySwap{
                    .sourceID = theScene.settings.clientUID, .destinationID = theScene.settings.clientUID, .myPlayerIndex = theScene.myPlayerIndex,
                    .sourceIndex = 0, .destinationIndex = (uint8_t)inv.getIndex(i, j), .mouseSlotS = true, .mouseSlotD = false});

                    // auto mouseslot = inv.mouseHeldItem;
                    // inv.mouseHeldItem = slot;
                    // slot = mouseslot;
                }
            }

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            {
                if (isEquip && !mouseHeldItemEquippable) {
                    ImGui::SetTooltip("Cannot equip this item.");
                } else if (isEquip && mouseHeldItemEquippable && inv.mouseHeldItem.block != 0) {
                    ImGui::SetTooltip("Equip this item.");
                } else if (!slot.empty()) {
                    if (slot.isItem) {
                        ImGui::SetTooltip(ToString(static_cast<ItemName>(slot.block)));
                    } else {
                        ImGui::SetTooltip(ToString(static_cast<MaterialName>(slot.block)));
                    }
                }
            }
            // if (isEquip)
            // {
            //     ImGui::PopStyleColor();
            // }
            ImGui::PopID();
        }
        ImGui::NewLine();
    }
}
