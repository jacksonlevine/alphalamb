//
// Created by jack on 1/27/2025.
//

#ifndef WORLDGENMETHOD_H
#define WORLDGENMETHOD_H
#include "../IntTup.h"
#include "../PrecompHeader.h"
enum Climate : uint8_t;
///The interface the world gen method must satisfy
class WorldGenMethod {
public:
    virtual BlockType get(IntTup spot) = 0;
    virtual float getHumidityNoise(const IntTup& spot) = 0;
    virtual float getTemperatureNoise(const IntTup& spot) = 0;
    virtual Climate getClimate(IntTup spot) = 0;
    virtual void setSeed(int seed) = 0;
    virtual int getSeed() = 0;
};
#endif //WORLDGENMETHOD_H
