//
// Created by jack on 1/31/2025.
//

#ifndef VOXELMODELREGISTRY_H
#define VOXELMODELREGISTRY_H

#include "../IntTup.h"
#include "../PrecompHeader.h"


struct VoxelPoint {
    IntTup localSpot;
    uint8_t colorIndex;
};

// New structure to hold both points and dimensions
struct VoxModel {
    std::vector<VoxelPoint> points;
    IntTup dimensions;  // Store x, y, z dimensions
};

struct RiffChunk {
    char id[4];
    uint32_t contentSize;
    uint32_t childrenSize;
};

extern std::vector<VoxModel> voxelModels;  // Modified to store VoxModel instead of just points

inline VoxModel loadVoxFile(const std::string& filename) {
    VoxModel model;
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        std::string loadError = "Cannot open file: ";
        loadError += filename;
        std::cout << loadError;
        //throw std::runtime_error(loadError);
    }

    char magic[4];
    file.read(magic, 4);
    if (std::string(magic, 4) != "VOX ") {
        throw std::runtime_error("Invalid VOX file format");
    }

    uint32_t version;
    file.read(reinterpret_cast<char*>(&version), 4);
    if (version != 150) {
        throw std::runtime_error("Unsupported VOX version");
    }

    RiffChunk mainChunk;
    file.read(reinterpret_cast<char*>(&mainChunk), sizeof(RiffChunk));
    if (std::string(mainChunk.id, 4) != "MAIN") {
        throw std::runtime_error("Missing MAIN chunk");
    }

    while (file.good()) {
        RiffChunk chunk;
        file.read(reinterpret_cast<char*>(&chunk), sizeof(RiffChunk));

        std::string chunkId(chunk.id, 4);

        if (chunkId == "SIZE") {
            uint32_t dimensions[3];
            file.read(reinterpret_cast<char*>(dimensions), 12);
            model.dimensions = IntTup(
                static_cast<int>(dimensions[0]),
                static_cast<int>(dimensions[2]),
                static_cast<int>(dimensions[1])
            );
            printf("Model dimensions: %d, %d, %d\n",
                model.dimensions.x,
                model.dimensions.y,
                model.dimensions.z);
        }
        else if (chunkId == "XYZI") {
            uint32_t numVoxels;
            file.read(reinterpret_cast<char*>(&numVoxels), 4);
            printf("Found %u voxels\n", numVoxels);

            model.points.reserve(numVoxels);
            for (uint32_t i = 0; i < numVoxels; i++) {
                uint8_t voxel[4];
                file.read(reinterpret_cast<char*>(voxel), 4);

                model.points.push_back({
                    IntTup(
                        static_cast<int>(voxel[0]),
                        static_cast<int>(voxel[2]),
                        static_cast<int>(voxel[1])),
                    voxel[3]
                });
            }
            break;
        } else {
            //skip this chunk
            file.seekg(chunk.contentSize + chunk.childrenSize, std::ios::cur);
        }
    }

    return model;
}

enum VoxelModelName
{
    Bush = 0,

    Tree1 = 1,
    Tree2 = 2,

    Rock1 = 3,
    Rock2 = 4,

    Tree3 = 5,
    Tree4 = 6,
    Tree5 = 7,

    Bamboo1 = 8,
    Bamboo2 = 9,

    TallGrass1 = 10,
    TallGrass2 = 11,
    TallGrass3 = 12,

    RubberTree = 13,
    PTree = 14,
    RedRock = 15,
    Crystal1 = 16,

    Awds = 17,

    PineTree1 = 18,
    PineTree2 = 19,

    ArticWillow = 20,

    CedarTree1 = 21,
    CedarTree2 = 22,

    PalmTree1 = 23,
    PalmTree2 = 24,
    PalmTree3 = 25,

    JoshuaTree1 = 26,
    JoshuaTree2 = 27,
    JoshuaTree3 = 28,

    PaperBirch1 = 29,
    PaperBirch2 = 30,
    PaperBirch3 = 31,

    GreenAlder1 = 32,
    GreenAlder2 = 33,

    Willow1 = 34,
    Willow2 = 35,
    Willow3 = 36,

    Beech1 = 37,
    Beech2 = 38,
    Beech3 = 39,

    WesternHemlock1 = 40,
    WesternHemlock2 = 41,
    WesternHemlock3 = 42,

    Eucalyptus1 = 43,
    Eucalyptus2 = 44,
    Eucalyptus3 = 45,

    Saguaro1 = 46,
    Saguaro2 = 47,
    Saguaro3 = 48,

    FigTree1 = 49,
    FigTree2 = 50,

    Pumpkin = 51,
};

inline void loadGameVoxelModels()
{
    voxelModels.insert(voxelModels.end(), {
        loadVoxFile("resources/voxelmodels/bush.vox"),
        loadVoxFile("resources/voxelmodels/tree1.vox"),
        loadVoxFile("resources/voxelmodels/tree2.vox"),
        loadVoxFile("resources/voxelmodels/rock1.vox"),
        loadVoxFile("resources/voxelmodels/rock2.vox"),
        loadVoxFile("resources/voxelmodels/tree3.vox"),
        loadVoxFile("resources/voxelmodels/tree4.vox"),
        loadVoxFile("resources/voxelmodels/tree5.vox"),
        loadVoxFile("resources/voxelmodels/bamboo1.vox"),
        loadVoxFile("resources/voxelmodels/bamboo2.vox"),
        loadVoxFile("resources/voxelmodels/tallgrass1.vox"),
        loadVoxFile("resources/voxelmodels/tallgrass2.vox"),
        loadVoxFile("resources/voxelmodels/tallgrass3.vox"),
        loadVoxFile("resources/voxelmodels/rubbertree.vox"),
        loadVoxFile("resources/voxelmodels/ptree.vox"),
        loadVoxFile("resources/voxelmodels/redrock.vox"),
        loadVoxFile("resources/voxelmodels/crystal1.vox"),
        loadVoxFile("resources/voxelmodels/awds.vox"),
        loadVoxFile("resources/voxelmodels/pinetree1.vox"),
        loadVoxFile("resources/voxelmodels/pinetree2.vox"),
        loadVoxFile("resources/voxelmodels/articwillow.vox"),
        loadVoxFile("resources/voxelmodels/cedartree1.vox"),
        loadVoxFile("resources/voxelmodels/cedartree2.vox"),
        loadVoxFile("resources/voxelmodels/palmtree1.vox"),
        loadVoxFile("resources/voxelmodels/palmtree2.vox"),
        loadVoxFile("resources/voxelmodels/palmtree3.vox"),
        loadVoxFile("resources/voxelmodels/joshuatree1.vox"),
        loadVoxFile("resources/voxelmodels/joshuatree2.vox"),
        loadVoxFile("resources/voxelmodels/joshuatree3.vox"),
        loadVoxFile("resources/voxelmodels/paperbirch1.vox"),
        loadVoxFile("resources/voxelmodels/paperbirch2.vox"),
        loadVoxFile("resources/voxelmodels/paperbirch3.vox"),
        loadVoxFile("resources/voxelmodels/greenalder1.vox"),
        loadVoxFile("resources/voxelmodels/greenalder2.vox"),
        loadVoxFile("resources/voxelmodels/willow1.vox"),
        loadVoxFile("resources/voxelmodels/willow2.vox"),
        loadVoxFile("resources/voxelmodels/willow3.vox"),
        loadVoxFile("resources/voxelmodels/beech1.vox"),
        loadVoxFile("resources/voxelmodels/beech2.vox"),
        loadVoxFile("resources/voxelmodels/beech3.vox"),
        loadVoxFile("resources/voxelmodels/westernhemlock1.vox"),
        loadVoxFile("resources/voxelmodels/westernhemlock2.vox"),
        loadVoxFile("resources/voxelmodels/westernhemlock3.vox"),
        loadVoxFile("resources/voxelmodels/eucalyptus1.vox"),
        loadVoxFile("resources/voxelmodels/eucalyptus2.vox"),
        loadVoxFile("resources/voxelmodels/eucalyptus3.vox"),
        loadVoxFile("resources/voxelmodels/saguaro1.vox"),
        loadVoxFile("resources/voxelmodels/saguaro2.vox"),
        loadVoxFile("resources/voxelmodels/saguaro3.vox"),
        loadVoxFile("resources/voxelmodels/figtree1.vox"),
        loadVoxFile("resources/voxelmodels/figtree2.vox"),
        loadVoxFile("resources/voxelmodels/pumpkin.vox"),
    });
}


#endif //VOXELMODELREGISTRY_H
