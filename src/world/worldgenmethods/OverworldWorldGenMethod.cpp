//
// Created by jack on 1/27/2025.
//

#include "OverworldWorldGenMethod.h"
#include "../MaterialName.h"

OverworldWorldGenMethod::OverworldWorldGenMethod()
{
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
}

uint32_t OverworldWorldGenMethod::get(IntTup spot)
{

    MaterialName floorBlock = getFloorBlockInClimate(getClimate(spot));

    float no = noise.GetNoise(
        spot.x * blockScaleInPerlin,
        spot.y * blockScaleInPerlin,
        spot.z * blockScaleInPerlin)
    - ((spot.y - 90.0) * 0.007);

    float noabove = noise.GetNoise(
        spot.x * blockScaleInPerlin,
        (spot.y + 2) * blockScaleInPerlin,
        spot.z * blockScaleInPerlin)
    - ((spot.y - 90.0) * 0.007);

    return no > 0.02f ? (
        noabove > 0.02f ? (DIRT) : (spot.y < 10 ? SAND : GRASS)
        ) : 0;


}

Climate OverworldWorldGenMethod::getClimate(IntTup spot)
{
    const float tempNoise = noise.GetNoise(
        (spot.x + 1000) * blockScaleInPerlin,
        (spot.y + 1000) * blockScaleInPerlin,
        spot.z * blockScaleInPerlin)
    * 2.0 - ((spot.y - 60) / 500.0f);

    const float humidityNoise = noise.GetNoise(
        spot.x * blockScaleInPerlin,
        spot.y * blockScaleInPerlin,
        spot.z * blockScaleInPerlin)
    * 2.0 - ((spot.y - 60) / 500.0f);

    const float tempClamped = glm::clamp(tempNoise, -1.0f, 1.0f);
    const float humClamped = glm::clamp(humidityNoise, -1.0f, 1.0f);

    const int tempIndex = std::min(2, static_cast<int>(std::floor((tempClamped + 1.0f) * 1.5f)));
    const int humIndex = std::min(2, static_cast<int>(std::floor((humClamped + 1.0f) * 1.5f)));

    const int index = tempIndex * 3 + humIndex;

    return static_cast<Climate>(index);
}

MaterialName OverworldWorldGenMethod::getFloorBlockInClimate(const Climate& climate)
{
    switch (climate)
    {
    case HotDesert:
        return SAND;
    case PolarDesert:
        return GRASS; //SNOWYGRASS
    default:
        return GRASS;
    }
}
