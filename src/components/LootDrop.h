//
// Created by jack on 5/15/2025.
//

#ifndef LOOTDROP_H
#define LOOTDROP_H
#include "../BlockType.h"

struct LootDrop
{
    int block = STONE;
    int count = 0;
    bool isItem = false;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(block, count, isItem);
    }
};

struct Scene;
struct World;

void renderLootDrops(entt::registry& reg, Scene* scene, float deltaTime);

#endif //LOOTDROP_H
