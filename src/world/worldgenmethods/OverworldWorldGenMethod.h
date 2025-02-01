//
// Created by jack on 1/27/2025.
//

#ifndef PERLINWORLDGENMETHOD_H
#define PERLINWORLDGENMETHOD_H

#include "../MaterialName.h"
#include "../WorldGenMethod.h"
#include "../VoxelModels.h"

enum Climate {
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
    uint32_t get(IntTup spot) override;
    Climate getClimate(IntTup spot);
    static MaterialName getFloorBlockInClimate(const Climate& climate);
    float getHumidityNoise(const IntTup& spot) override;
    float getTemperatureNoise(const IntTup& spot) override;
private:
    FastNoiseLite noise = {};
    static constexpr float blockScaleInPerlin = 1.7f;
    static constexpr float tempNoiseScale = 0.2f;
    static constexpr float humidityNoiseScale = 0.4f;
    static constexpr float beachNoiseScale = 2.7f;
};





#endif //PERLINWORLDGENMETHOD_H
