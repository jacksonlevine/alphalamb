//
// Created by jack on 2/20/2025.
//

#ifndef TIMEOFDAY_H
#define TIMEOFDAY_H
#include "PrecompHeader.h"

inline float gaussian(float x, float peak, float radius) {
    float stdDev = radius / 3.0;
    float variance = stdDev * stdDev;

    float b = exp(-pow(x - peak, 2.0) / (2.0 * variance));

    float peakHeight = exp(-pow(peak - peak, 2.0) / (2.0 * variance));
    return b / peakHeight;
};

inline float ambBrightFromTimeOfDay(float timeOfDay, float dayLength)
{
    return std::max(0.05f, std::min(1.0f, gaussian(timeOfDay, dayLength/1.75f, dayLength/2.0f) * 1.3f));
}


#endif //TIMEOFDAY_H
