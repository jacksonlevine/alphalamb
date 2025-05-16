//
// Created by jack on 5/15/2025.
//

#ifndef LOOTDROP_H
#define LOOTDROP_H
#include "../BlockType.h"

struct LootDrop
{
    BlockType block = STONE;
    int count = 0;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(block, count);
    }
};

struct Scene;

void renderLootDrops(entt::registry& reg, Scene* scene);

#endif //LOOTDROP_H
