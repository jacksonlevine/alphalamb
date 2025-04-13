//
// Created by jack on 4/12/2025.
//

#ifndef LIGHT_H
#define LIGHT_H

#include "IntTup.h"
#include "PrecompHeader.h"
#include "world/World.h"


struct LightRay {
    uint32_t originhash;
    int level;
};

struct LightSpot {
    std::vector<LightRay> rays;
    int sum() {
        int res = 0;
        for(auto ray : rays) {
            res += ray.level;
        }
        return res;
    }
};

constexpr auto neighbs = std::to_array({
    IntTup(-1,0,0), IntTup(1,0,0),
    IntTup(0,-1,0), IntTup(0,1,0),
    IntTup(0,0,-1), IntTup(0,0,1)
});

void setLightLevelFromOriginHere(IntTup spot, IntTup origin, int value, std::unordered_map<IntTup, LightSpot, IntTupHash>& lightmap);

std::vector<std::pair<IntTup, int>> getChunkLightSources(const TwoIntTup& spot, World* world, int chunkw, int chunkh, std::unordered_map<IntTup, LightSpot, IntTupHash>& lightmap, bool
                                                         locked);

void propagateAllLightsLayered(World* world, const std::vector<std::pair<IntTup, int>>& lightSources,
                               std::unordered_map<IntTup, LightSpot, IntTupHash>& lightmap,
                               std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks = nullptr, bool
                               locked = false);


void unpropagateAllLightsLayered(const std::vector<std::pair<IntTup, int>>& lightSourcesToRemove, std::unordered_map<IntTup, LightSpot,
                                IntTupHash>& lightmap, std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks = nullptr, bool locked = false);


void lightPassOnChunk(World* world, TwoIntTup spot, int chunkw, int chunkh, std::unordered_map<IntTup, LightSpot, IntTupHash>& lightmap, std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks = nullptr, bool
                      locked = false);





#endif //LIGHT_H
