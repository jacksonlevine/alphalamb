//
// Created by jack on 1/27/2025.
//

#ifndef PERLINWORLDGENMETHOD_H
#define PERLINWORLDGENMETHOD_H

#include "../MaterialName.h"
#include "../WorldGenMethod.h"

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


///A simple implementation of WorldGenMethod using Perlin noise
class OverworldWorldGenMethod : public WorldGenMethod {
public:
    OverworldWorldGenMethod();
    uint32_t get(IntTup spot) override;
    Climate getClimate(IntTup spot);
    static MaterialName getFloorBlockInClimate(const Climate& climate);
private:
    FastNoiseLite noise = {};
    static constexpr float blockScaleInPerlin = 1.7f;
    static constexpr float tempNoiseScale = 0.002f;
    static constexpr float humidityNoiseScale = 0.004f;
    static constexpr float beachNoiseScale = 0.13f;
};





#endif //PERLINWORLDGENMETHOD_H
