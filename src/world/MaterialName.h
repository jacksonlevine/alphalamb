//
// Created by jack on 1/27/2025.
//

#ifndef MATERIALNAME_H
#define MATERIALNAME_H

#include "../BlockType.h"
#include "../PrecompHeader.h"
#include "../EnumToString.h"
constexpr int BLOCK_COUNT = 93;
//there is a value in the shader that needs to be updated when this is changed. annoying, fix soon

DEFINE_ENUM_WITH_STRING_CONVERSIONS(MaterialName, BlockType,
    (AIR)
    (SAND)
    (WATER)
    (GRASS)
    (DIRT)
    (COBBLESTONE)
    (WOOD)
    (LEAVES)
    (GLASS)
    (STONE)
    (WOOD_PLANKS)
    (BUSH_LEAVES)
    (PETRIFIED_WOOD)
    (RED_STONE)
    (SALTED_EARTH)
    (BEDROCK)
    (STONE_STAIRS)
    (WOOD_STAIRS)
    (LIGHT)
    (DOOR)
    (LADDER)
    (WOODEN_TRUNK)
    (BAMBOO)
    (TALL_GRASS)
    (BLUE_LIGHT)
    (PURPLE_LIGHT)
    (YELLOW_LIGHT)
    (RED_LIGHT)
    (GREEN_LIGHT)
    (ORANGE_LIGHT)
    (TEAL_LIGHT)
    (CRAFTING_BENCH)
    (APPLE)
    (BAMBOO_PIECE)
    (DEAD_LEAF_MULCH)
    (METAL_ROCK)
    (CRUDE_BLADE)
    (CRUDE_PICK)
    (CRUDE_MATTOCK)
    (CRUDE_AXE)
    (JUMP_SWITCHER_BLOCK)
    (JUMP_SWITCHER_BLOCK2)
    (TRAMPOLINE_BLOCK)
    (RUBBER_TREE_WOOD)
    (RUBBER_TREE_LEAVES)
    (CONVEYOR)
    (AUTO_TRAMPOLINE)
    (METAL_PLATE_BLOCK)
    (SNOWY_GRASS_BLOCK)
    (TORCH)
    (SNOWY_LEAVES)
    (ICE)
    (ARTIC_WILLOW_DWARF_SHRUB)
    (PINE_WOOD)
    (PINE_LEAVES)
    (ARTIC_WILLOW_LEAVES)
    (CEDAR_WOOD)
    (CEDAR_LEAVES)
    (PALM_WOOD)
    (PALM_LEAVES)
    (JOSHUA_WOOD)
    (JOSHUA_LEAVES)
    (SNOWY_SAND)
    (FENCE)
    (PAPER_BIRCH_WOOD)
    (PAPER_BIRCH_LEAVES)
    (GREEN_ALDER_WOOD)
    (GREEN_ALDER_LEAVES)
    (WILLOW_WOOD)
    (WILLOW_LEAVES)
    (BEECH_WOOD)
    (BEECH_LEAVES)
    (WESTERN_HEMLOCK_WOOD)
    (WESTERN_HEMLOCK_LEAVES)
    (EUCALYPTUS_WOOD)
    (EUCALYPTUS_LEAVES)
    (FIG_WOOD)
    (FIG_LEAVES)
    (SAGUARO_BLOCK)
    (PUMPKIN)
    (JACK_O_LANTERN)
    (PINE_PLANKS)
    (CEDAR_PLANKS)
    (COBBLESTONE_STAIRS)
    (JOSHUA_PLANKS)
    (BIRCH_PLANKS)
    (WILLOW_PLANKS)
    (BEECH_PLANKS)
    (WESTERN_HEMLOCK_PLANKS)
    (ORANGE1_DUMMY_BLOCK)
    (CABLE)
    (JETPACK_PARTICLE_BLOCK)
    (DG_COMPUTERBLOCK)
)

constexpr std::bitset<BLOCK_COUNT> makeBitset(std::initializer_list<int> ids) {
    std::bitset<BLOCK_COUNT> bits;
    for (int id : ids) {
        bits.set(id);
    }
    return bits;
}

constexpr auto noHeadBlock = makeBitset({DOOR, TALL_GRASS, ORANGE1_DUMMY_BLOCK});

constexpr auto noAmbOccl = makeBitset({FENCE, GLASS, DG_COMPUTERBLOCK, DOOR, CABLE, LIGHT, TALL_GRASS, ORANGE1_DUMMY_BLOCK});

constexpr auto noCustCollShape = makeBitset({DG_COMPUTERBLOCK});

constexpr auto liquids = makeBitset({WATER});

constexpr auto noColl = makeBitset({WATER, TALL_GRASS, BAMBOO, ORANGE1_DUMMY_BLOCK});

constexpr auto trulynothing = makeBitset({AIR, ORANGE1_DUMMY_BLOCK});
constexpr auto grasstypes = makeBitset({TALL_GRASS, GRASS});

constexpr auto marchers = makeBitset({GRASS, DIRT, SAND});
constexpr auto transparents = makeBitset({
    ORANGE1_DUMMY_BLOCK,
    AIR,
    DG_COMPUTERBLOCK,
    STONE_STAIRS,
    WOOD_STAIRS,
    LEAVES,
    BUSH_LEAVES,
    LIGHT,
    DOOR,
    LADDER,
    WOODEN_TRUNK,
    BAMBOO,
    RED_LIGHT,
    SNOWY_LEAVES,
    ARTIC_WILLOW_LEAVES,
    CEDAR_LEAVES,
    PALM_LEAVES,
    JOSHUA_LEAVES,
    PAPER_BIRCH_LEAVES,
    GREEN_ALDER_LEAVES,
    WILLOW_LEAVES,
    BEECH_LEAVES,
    WESTERN_HEMLOCK_LEAVES,
    EUCALYPTUS_LEAVES,
    FIG_LEAVES,
    JACK_O_LANTERN,
    CABLE,
    WATER,
    GLASS,
    TORCH,
    FENCE,
    TALL_GRASS,
    ARTIC_WILLOW_DWARF_SHRUB
});

using TexCoord = std::pair<uint8_t, uint8_t>;
using BlockTextures = std::array<TexCoord, 3>;

constexpr std::array<BlockTextures, BLOCK_COUNT> TEXS = {{
    // sides      // bot       // top
    {{ {0, 0}, {0, 0}, {0, 0} }},  // 0
    {{ {1, 0}, {1, 0}, {1, 0} }},  // 1 sand
    {{ {2, 0}, {2, 0}, {2, 0} }},  // 2 water
    {{ {3, 0}, {4, 0}, {3, 1} }},  // 3 grass
    {{ {4, 0}, {4, 0}, {4, 0} }},  // 4 dirt
    {{ {5, 0}, {5, 0}, {5, 0} }},  // 5 cobble
    {{ {6, 0}, {6, 1}, {6, 1} }},  // 6 log
    {{ {7, 0}, {7, 0}, {7, 0} }},  // 7 leaves
    {{ {8, 0}, {8, 0}, {8, 0} }},  // 8 glass
    {{ {9, 0}, {9, 0}, {9, 0} }},  // 9 smooth stone
    {{ {10, 0}, {10, 0}, {10, 0} }}, // 10 planks wood
    {{ {7, 1}, {7, 1}, {7, 1} }}, // 11 bush leaves
    {{ {4, 2}, {4, 2}, {4, 2} }}, // 12 petrified wood
    {{ {6, 2}, {6, 2}, {6, 2} }}, // 13 red stone
    {{ {7, 2}, {7, 2}, {7, 2} }}, // 14 salted earth
    {{ {8, 2}, {8, 2}, {8, 2} }}, // 15 bedrock
    {{ {9, 0}, {9, 0}, {9, 0} }}, // 16 stone stairs
    {{ {10, 0}, {10, 0}, {10, 0} }}, // 17 wood stairs
    {{ {12, 1}, {12, 1}, {12, 1} }}, // 18 light
    {{ {12, 0}, {12, 0}, {12, 0} }}, // 19 door
    {{ {0, 1}, {0, 1}, {0, 1} }}, // 20 ladder
    {{ {14, 2}, {14, 2}, {14, 2} }}, // 21 wooden trunk
    {{ {13, 1}, {14, 1}, {14, 1} }}, // 22 bamboo
    {{ {1, 3}, {1, 3}, {1, 3} }}, // 23 tallgrass
    {{ {10, 2}, {10, 2}, {10, 2} }}, // 24 blue light
    {{ {11, 2}, {11, 2}, {11, 2} }}, // 25 purple light
    {{ {12, 2}, {12, 2}, {12, 2} }}, // 26 yellow light
    {{ {13, 2}, {13, 2}, {13, 2} }}, // 27 red light
    {{ {10, 3}, {10, 3}, {10, 3} }}, // 28 green light
    {{ {11, 3}, {11, 3}, {11, 3} }}, // 29 orange light
    {{ {12, 3}, {12, 3}, {12, 3} }}, // 30 teal light
    {{ {1, 5}, {1, 5}, {1, 5} }}, // 31 crafttable
    {{ {3, 3}, {3, 3}, {3, 3} }}, // 32 apple
    {{ {2, 3}, {2, 3}, {2, 3} }}, // 33 bamboo chute
    {{ {7, 4}, {7, 4}, {7, 4} }}, // 34 dead leaves
    {{ {2, 4}, {2, 4}, {2, 4} }}, // 35 metal rock
    {{ {2, 5}, {2, 5}, {2, 5} }}, // 36 crude blade
    {{ {3, 5}, {3, 5}, {3, 5} }}, // 37 crude pick
    {{ {4, 5}, {4, 5}, {4, 5} }}, // 38 crude mattock
    {{ {5, 5}, {5, 5}, {5, 5} }}, // 39 crude axe
    {{ {10, 4}, {10, 4}, {10, 4} }}, // 40 jumper blue
    {{ {11, 4}, {11, 4}, {11, 4} }}, // 41 jumper yellow
    {{ {10, 5}, {10, 5}, {10, 5} }}, // 42 trampoline block
    {{ {0, 8}, {2, 8}, {2, 8} }}, // 43 rubber tree wood
    {{ {1, 8}, {1, 8}, {1, 8} }}, // 44 rubber tree leaves
    {{ {10, 6}, {10, 6}, {10, 6} }}, // 45 conveyor/highway
    {{ {11, 5}, {11, 5}, {11, 5} }}, // 46 auto trampoline block
    {{ {1, 6}, {1, 6}, {1, 6} }}, // 47 metal plate block
    {{ {8, 4}, {4, 0}, {8, 5} }}, // 48 snowy grass
    {{ {9, 4}, {9, 4}, {9, 4} }}, // 49 torch
    {{ {7, 5}, {7, 0}, {8, 5} }}, // 50 snow leaves
    {{ {1, 7}, {1, 7}, {1, 7} }}, // 51 ice
    {{ {1, 12}, {1, 12}, {1, 12} }}, // 52 Artic Willow Dwarf Shrub
    {{ {3, 6}, {3, 7}, {3, 7} }}, // 53 Pine Wood
    {{ {3, 8}, {3, 8}, {3, 8} }}, // 54 Pine Leaves
    {{ {2, 12}, {2, 12}, {2, 12} }}, // 55 Artic Willow Leaves
    {{ {4, 6}, {4, 7}, {4, 7} }}, // 56 Cedar Wood
    {{ {4, 8}, {4, 8}, {4, 8} }}, // 57 Cedar Leaves
    {{ {5, 6}, {5, 7}, {5, 7} }}, // 58 Palm Wood
    {{ {5, 8}, {5, 8}, {5, 8} }}, // 59 Palm Leaves
    {{ {6, 6}, {6, 7}, {6, 7} }}, // 60 Joshua Wood
    {{ {6, 8}, {6, 8}, {6, 8} }}, // 61 Joshua Leaves
    {{ {0, 9}, {1, 0}, {8, 5} }}, // 62 snowy sand
    {{ {13, 3}, {13, 3}, {13, 3} }}, // 63 fence
    {{ {7, 6}, {7, 7}, {7, 7} }}, // 64 Paper Birch Wood
    {{ {7, 8}, {7, 8}, {7, 8} }}, // 65 Paper Birch Leaves
    {{ {8, 6}, {8, 7}, {8, 7} }}, // 66 Green Alder Wood
    {{ {8, 8}, {8, 8}, {8, 8} }}, // 67 Green Alder Leaves
    {{ {9, 6}, {9, 7}, {9, 7} }}, // 68 Willow Wood
    {{ {9, 8}, {9, 8}, {9, 8} }}, // 69 willow Leaves
    {{ {3, 9}, {3, 10}, {3, 10} }}, // 70 Beech Wood
    {{ {3, 11}, {3, 11}, {3, 11} }}, // 71 Beech Leaves
    {{ {4, 9}, {4, 10}, {4, 10} }}, // 72 Western Hemlock Wood
    {{ {4, 11}, {4, 11}, {4, 11} }}, // 73 Western Hemlock Leaves
    {{ {5, 9}, {5, 10}, {5, 10} }}, // 74 Eucalyptus Wood
    {{ {5, 11}, {5, 11}, {5, 11} }}, // 75 Eucalyptus Leaves
    {{ {6, 9}, {6, 10}, {6, 10} }}, // 76 Fig Wood
    {{ {6, 11}, {6, 11}, {6, 11} }}, // 77 Fig Leaves
    {{ {7, 9}, {7, 10}, {7, 10} }}, // 78 Saguaro Block
    {{ {3, 12}, {4, 13}, {3, 13} }}, // 79 Pumpkin
    {{ {4, 12}, {4, 13}, {3, 13} }}, // 80 Jack o lantern
    {{ {5, 12}, {5, 12}, {5, 12} }}, // 81 Pine Planks
    {{ {6, 12}, {6, 12}, {6, 12} }}, // 82 Cedar Planks
    {{ {7, 12}, {7, 12}, {7, 12} }}, // 83 Palm Planks
    {{ {8, 12}, {8, 12}, {8, 12} }}, // 84 Joshua Planks
    {{ {9, 12}, {9, 12}, {9, 12} }}, // 85 Birch Planks
    {{ {10, 12}, {10, 12}, {10, 12} }}, // 86 Willow Planks
    {{ {11, 12}, {11, 12}, {11, 12} }}, // 87 Beech Planks
    {{ {12, 12}, {12, 12}, {12, 12} }}, // 88 Western Hemlock Planks
    {{ {13, 12}, {13, 12}, {13, 12} }}, // 89 orange1 dummy block
    {{ {12, 4}, {12, 4}, {12, 4} }}, // 90 Cable
    {{ {14, 14}, {14, 14}, {14, 14} }}, // 91 Jetback Particle Block
        {{ {14,3}, {14,4}, {14,4} }} // 92 Computer
}};


constexpr std::array<BlockTextures, 2> ITEMTEXS = {{
    // sides      // bot       // top
    {{ {0, 0}, {0, 0}, {0, 0} }},  // 0
    {{ {1, 0}, {1, 0}, {1, 0} }},  // 1 jetpack

}};

#endif //MATERIALNAME_H
