//
// Created by jack on 5/10/2025.
//

#ifndef HELPERS_H
#define HELPERS_H

#include "IntTup.h"
#include "jl484_vec3.h"
inline TwoIntTup worldToChunkPos(const TwoIntTup& worldPos)
{
    return {
        static_cast<int>(std::floor(static_cast<float>(worldPos.x) / 16)),
        static_cast<int>(std::floor(static_cast<float>(worldPos.z) / 16))
    };
}

inline TwoIntTup world3ToChunkPos(const IntTup& worldPos)
{
    return {
        static_cast<int>(std::floor(static_cast<float>(worldPos.x) / 16)),
        static_cast<int>(std::floor(static_cast<float>(worldPos.z) / 16))
    };
}

inline jl484_vec3 worldToChunkLocalPos(const IntTup& worldPos) {
    return jl484_vec3(
        (worldPos.x % 16 + 16) % 16,  // X: [0, 15]
        worldPos.y % 256,             // Y: [0, 255]
        (worldPos.z % 16 + 16) % 16   // Z: [0, 15]
    );
}
#endif //HELPERS_H
