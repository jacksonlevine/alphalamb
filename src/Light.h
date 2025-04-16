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


constexpr int MAXLIGHTVAL = 12;
constexpr int TORCHLIGHTVAL = 10;

struct LightRay {
    uint32_t originhash;
    int level;
};

struct LightSpot {
    boost::container::vector<LightRay> rays;
    int sum() {
        int res = 0;
        for(auto ray : rays) {
            res += ray.level;
        }
        return glm::min(12, res);
    }
};
using LightMapType = boost::unordered_flat_map<IntTup, LightSpot, IntTupHash>;

constexpr auto neighbs = std::to_array({
    IntTup(-1,0,0), IntTup(1,0,0),
    IntTup(0,-1,0), IntTup(0,1,0),
    IntTup(0,0,-1), IntTup(0,0,1)
});


constexpr auto neighbs4 = std::to_array({
    IntTup(-1,0,0), IntTup(1,0,0),
    IntTup(0,0,-1), IntTup(0,0,1)
});

template<bool ifHigher = false>
void setLightLevelFromOriginHere(IntTup spot, IntTup origin, int value,
                                 LightMapType& lightmap)
{
    auto lmentry = lightmap.find(spot);
    auto originhash = IntTupHash{}(origin, true);
    if (lmentry != lightmap.end()) {
        auto& rays = lmentry->second.rays;

        if (value != 0) {
            bool originfound = false;

            for (auto& ray : rays) {
                if (ray.originhash == originhash) {
                    originfound = true;
                    if constexpr (ifHigher) {
                        if (ray.level < value) {
                            ray.level = value;
                        }
                    } else {
                        ray.level = value;
                    }
                    break;
                }
            }
            if (!originfound) {
                rays.push_back(LightRay{
                    .originhash = originhash, .level = value
                });
            }
        } else {
            rays.erase(
                std::remove_if(
                    rays.begin(),
                    rays.end(),
                    [originhash](const auto& ray) { return ray.originhash == originhash; }
                ),
                rays.end()
            );
            if (rays.empty()) {
                lightmap.erase(lmentry);
            }
        }
    } else if (value != 0) { // Only add new entry if value != 0
        lightmap.insert({spot, LightSpot{}});
        lightmap.at(spot).rays.push_back(LightRay{
            .originhash = originhash, .level = value
        });
    }
}

struct ChunkLightSources
{
    std::vector<std::pair<IntTup, int>> oldBlockSources;
    std::vector<std::pair<IntTup, int>> newBlockSources;
    std::vector<std::pair<IntTup, int>> ambientOldSources;
    std::vector<std::pair<IntTup, int>> ambientNewSources;
};


ChunkLightSources getChunkLightSourcesBlockAndAmbient(
    const TwoIntTup& spot, World* world, int chunkw, int chunkh, bool
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
