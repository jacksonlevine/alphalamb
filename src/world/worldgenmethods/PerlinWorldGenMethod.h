//
// Created by jack on 1/27/2025.
//

#ifndef PERLINWORLDGENMETHOD_H
#define PERLINWORLDGENMETHOD_H

#include "../WorldGenMethod.h"

///A simple implementation of WorldGenMethod using Perlin noise
class PerlinWorldGenMethod : public WorldGenMethod {
public:
    PerlinWorldGenMethod();
    uint32_t get(IntTup spot) override;
private:
    FastNoiseLite noise = {};
    float blockScaleInPerlin = 1.7f;
};



#endif //PERLINWORLDGENMETHOD_H
