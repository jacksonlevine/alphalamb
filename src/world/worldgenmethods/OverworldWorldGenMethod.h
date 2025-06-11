//
// Created by jack on 1/27/2025.
//

#ifndef PERLINWORLDGENMETHOD_H
#define PERLINWORLDGENMETHOD_H

#include "../MaterialName.h"
#include "../WorldGenMethod.h"
#include "../VoxelModels.h"
#include "../../BlockType.h"

enum Climate : uint8_t {
    PolarDesert,
    BorealForest,
    WetTundra,
    TemperateGrassland,
    DeciduousForest,
    TemperateRainforest,
    HotDesert,
    Savannah,
    TropicalRainforest,
};

enum TerrainFeature
{
    ArticWillowDwarfShrub,
    Pine,
    ArticWillowFeat,
    Oak,
    Maple,
    Cedar,
    Palm,
    Joshua,
    Rubber,
    PaperBirch,
    GreenAlder,
    Willow,
    Beech,
    WesternHemlock,
    Eucalyptus,
    Saguaro,
    FigTree,
    PumpkinFeat,
    Boulder,
    Bamboo,
    LightberryBush,
};

extern std::vector<std::vector<VoxelModelName>> terrainFeatureModels;

inline std::vector<VoxelModelName>& getVoxelModelNamesFromTerrainFeatureName(TerrainFeature& feat)
{
    return terrainFeatureModels.at(feat);
}

extern std::vector<std::vector<TerrainFeature>> climateTerrainFeatures;

inline std::vector<TerrainFeature>& getTerrainFeaturesFromClimate(Climate& climate)
{
    return climateTerrainFeatures[climate];
}

///A simple implementation of WorldGenMethod using Perlin noise
class OverworldWorldGenMethod : public WorldGenMethod {
public:
    OverworldWorldGenMethod();
    BlockType get(IntTup spot) override;
    Climate getClimate(IntTup spot) override;
    void setSeed(int seed) override;
    static MaterialName getFloorBlockInClimate(const Climate& climate);
    float getHumidityNoise(const IntTup& spot) override;
    float getTemperatureNoise(const IntTup& spot) override;
    float getNoiseMix(float x, float y, float z);
    int getSeed() override;
private:
    FastNoiseLite noise = {};
    static constexpr float blockScaleInPerlin = 0.35f;
    static constexpr float tempNoiseScale = 0.01f;
    static constexpr float humidityNoiseScale = 0.15f;
    static constexpr float beachNoiseScale = 1.7f;
};





#endif //PERLINWORLDGENMETHOD_H
