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
  ;

    return no > 0.02f ? STONE : AIR;
}
