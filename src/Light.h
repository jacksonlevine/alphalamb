//
// Created by jack on 4/12/2025.
//

#ifndef LIGHT_H
#define LIGHT_H

#include "IntTup.h"
#include "PrecompHeader.h"
#include "world/World.h"

//struct Light
extern boost::sync_queue<TwoIntTup> lightOverlapNotificationQueue;


struct LightRay {
    IntTup origin;
    int level;
};

struct LightSpot {
    boost::container::vector<LightRay> rays;
    int sum() {
        int res = 0;
        for(auto ray : rays) {
            res += ray.level;
        }
        return glm::min(16, res);
    }
};
using LightMapType = boost::unordered_flat_map<IntTup, LightSpot, IntTupHash>;

constexpr auto neighbs = std::to_array({
    IntTup(-1,0,0), IntTup(1,0,0),
    IntTup(0,-1,0), IntTup(0,1,0),
    IntTup(0,0,-1), IntTup(0,0,1)
});

template<bool ifHigher = false>
void setLightLevelFromOriginHere(IntTup spot, IntTup origin, int value,
                                 LightMapType& lightmap)
{
    auto lmentry = lightmap.find(spot);
    bool originfound = false;
    if(lmentry != lightmap.end()) {
        if(value != 0) {
            for(auto ray : lmentry->second.rays) {
                if(ray.origin == origin) {
                    originfound = true;
                    if constexpr (ifHigher)
                    {
                        if (ray.level < value) ray.level = value;

                    } else
                    {
                        ray.level = value;
                    }

                    break;
                }
            }
            if(!originfound) {
                lmentry->second.rays.push_back(LightRay{
                    .origin = origin, .level = value
                });
            }
        } else {
            lmentry->second.rays.erase(
                std::remove_if(
                    lmentry->second.rays.begin(),
                    lmentry->second.rays.end(),
                    [origin](auto & ray) { return ray.origin == origin; }
                ),
                lmentry->second.rays.end()
            );
        }
    } else {
        lightmap.insert({spot, LightSpot{}});
        lightmap.at(spot).rays.push_back(LightRay{
            .origin = origin, .level = value
        });
    }
}
std::pair<std::vector<std::pair<IntTup, int>>, std::vector<std::pair<IntTup, int>>> getChunkLightSourcesBlockAndAmbient(
    const TwoIntTup& spot, World* world, int chunkw, int chunkh, LightMapType&
    lightmap, bool
    locked);

void propagateAllLightsLayered(World* world, const std::vector<std::pair<IntTup, int>>& lightSources,
                               LightMapType& lightmap, TwoIntTup chunkOrigin,
                               std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks = nullptr, bool
                               locked = false);


void unpropagateAllLightsLayered(const std::vector<std::pair<IntTup, int>>& lightSourcesToRemove, LightMapType&
                                 lightmap, TwoIntTup chunkOrigin, std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks = nullptr, bool locked = false);

template<bool queue = true>
void lightPassOnChunk(World* world, TwoIntTup spot, int chunkw, int chunkh,
                      bool
                      locked);

inline float getBlockAmbientLightVal(uint16_t value1, uint16_t value2)
{
    uint32_t packed = (uint32_t(value1) << 16) | uint32_t(value2);

    float packedFloat;
    memcpy(&packedFloat, &packed, sizeof(float));
    return packedFloat;
}


#endif //LIGHT_H
