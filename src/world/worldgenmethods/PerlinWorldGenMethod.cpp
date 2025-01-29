//
// Created by jack on 1/27/2025.
//

#include "PerlinWorldGenMethod.h"
#include "../MaterialName.h"

PerlinWorldGenMethod::PerlinWorldGenMethod()
{
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
}

uint32_t PerlinWorldGenMethod::get(IntTup spot)
{

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
