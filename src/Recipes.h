//
// Created by jack on 6/11/2025.
//

#ifndef RECIPES_H
#define RECIPES_H


#include "Inventory.h"
#include "PrecompHeader.h"


using SpecificOrNonSpecificMaterial = std::variant<MaterialName, ItemName, std::bitset<BLOCK_COUNT>>;

struct Requirement
{
    SpecificOrNonSpecificMaterial mat;
    uint8_t count;
    bool isItem : 1 = false;
};

struct RecipeOutput
{
    int mat;
    uint8_t count;
    bool isItem: 1 = false;
};

using Recipe = std::pair<std::vector<Requirement>, RecipeOutput>;

inline std::array recipes = {
    Recipe{ {{BUSH_LEAVES, 1}}  ,   {LIGHT, 1} },
    Recipe{ {{TRUNKSTUFF, 1}}  ,   {WOOD_PLANKS, 4} },
    Recipe{ {{WOOD_PLANKS, 4}}  ,   {DOOR, 1} },
    Recipe{ {{STONE, 4}}  ,   {STONE_STAIRS, 1} },
    Recipe{ {{COBBLESTONE, 4}}  ,   {COBBLESTONE_STAIRS, 1} },
    Recipe{ {{WOOD_PLANKS, 4}}  ,   {WOOD_STAIRS, 1} },
    Recipe{ {{WOOD_PLANKS, 2}}  ,   {FENCE, 4} },
    Recipe{ {{SAND, 1}}  ,   {GLASS, 4} },
};


inline bool canMakeShit(Inventory& inv, const Recipe& recipe)
{
    bool missingsomething = false;
    for (const auto req : recipe.first)
    {
        bool reqfound = false;


        for (const auto& r : inv.inventory)
        {
            if (r.count >= req.count)
            {
                std::visit([&r, &reqfound, &req](const auto& mat)
                {
                    using T = std::decay_t<decltype(mat)>;
                    if constexpr (std::is_same_v<T, MaterialName>)
                    {
                        if (mat == r.block && r.isItem == req.isItem)
                        {
                            reqfound = true;
                        }
                    } else if constexpr (std::is_same_v<T, std::bitset<BLOCK_COUNT>>)
                    {
                        if (mat.test(r.block) && r.isItem == req.isItem)
                        {
                            reqfound = true;
                        }
                    }
                }, req.mat);
            }

            if (reqfound)
            {
                break;
            }

        }
        if (!reqfound)
        {
            missingsomething = true;
            break;
        }
    }
    return !missingsomething;
}

inline std::vector<int> getShitCanMake(Inventory& inv)
{
    std::vector<int> recs;

    int index = 0;
    for (const auto& recipe : recipes)
    {
        bool canmake = canMakeShit(inv, recipe);
        if (canmake)
        {
            recs.push_back(index);
        }
        index++;
    }
    return recs;
}

inline bool doRecipeOnInv(InventoryComponent& inv, int recipeIndex)
{
    auto& recipe = recipes.at(recipeIndex);
    if (canMakeShit(inv.inventory, recipe))
    {
        for (auto& req : recipe.first)
        {
            for (auto& slot : inv.inventory.inventory)
            {
                if (slot.count >= req.count)
                {
                    std::visit([&req, &slot](const auto& mat)
                    {
                        using T = std::decay_t<decltype(mat)>;
                        if constexpr (std::is_same_v<T, MaterialName>)
                        {
                            if (mat == slot.block)
                            {
                                slot.count -= req.count;
                                if (slot.count == 0)
                                {
                                    slot.block = 0;
                                }
                            }
                        } else if constexpr (std::is_same_v<T, std::bitset<BLOCK_COUNT>>)
                        {
                            if (mat.test(slot.block))
                            {
                                slot.count -= req.count;
                                if (slot.count == 0)
                                {
                                    slot.block = 0;
                                }
                            }
                        }
                    }, req.mat);
                }
            }
        }
        inv.add(LootDrop{recipe.second.mat, recipe.second.count, recipe.second.isItem});
        return true;
    }
    return false;
}


#endif //RECIPES_H
