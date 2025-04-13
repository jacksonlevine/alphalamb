//
// Created by jack on 4/12/2025.
//

#include "Light.h"

void setLightLevelFromOriginHere(IntTup spot, IntTup origin, int value,
    std::unordered_map<IntTup, LightSpot, IntTupHash>& lightmap)
{
    auto lmentry = lightmap.find(spot);
    bool originfound = false;
    auto originhash = IntTupHash{}(origin, true);
    if(lmentry != lightmap.end()) {
        if(value != 0) {
            for(auto ray : lmentry->second.rays) {
                if(ray.origin == originhash) {
                    originfound = true;
                    ray.level = value;
                    break;
                }
            }
            if(!originfound) {
                lmentry->second.rays.push_back(LightRay{
                    .origin = originhash, .level = value
                });
            }
        } else {
            std::erase_if(lmentry->second.rays, [originhash](auto & ray) { return ray.origin == originhash; });
        }
    } else {
        lightmap.insert({spot, LightSpot{}});
        lightmap.at(spot).rays.push_back(LightRay{
            .origin = originhash, .level = value
        });
    }
}

std::vector<std::pair<IntTup, int>> getChunkLightSources(const TwoIntTup& spot, World* world, int chunkw, int chunkh,
                                                         std::unordered_map<IntTup, LightSpot, IntTupHash>& lightmap, bool locked)
{
    std::vector<std::pair<IntTup, int>> chunkLightSources;
    std::shared_lock<std::shared_mutex> url;
    std::shared_lock<std::shared_mutex> nrl;


    if (!locked)
    {
        url = std::shared_lock<std::shared_mutex>(world->userDataMap->mutex());
        nrl = std::shared_lock<std::shared_mutex>(world->nonUserDataMap->mutex());
    }
    auto startspot = IntTup(spot.x * chunkw, 0, spot.z * chunkw);
    for (int i = 0; i < chunkw; i++)
    {
        for (int k = 0; k < chunkw; k++)
        {
            for (int j = 0; j < chunkh; j++)
            {
                auto spott = startspot + IntTup(i,j,k);
                if (world->getLocked(spott) == LIGHT)
                {
                    chunkLightSources.push_back(std::make_pair(spott, 12));
                }
            }
        }

    }
    return chunkLightSources;
}

void propagateAllLightsLayered(World* world, const std::vector<std::pair<IntTup, int>>& lightSources,
                               std::unordered_map<IntTup, LightSpot, IntTupHash>& lightmap, std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks, bool
                               locked)
{
    constexpr int maxLightLevel = 16;

    std::unordered_set<IntTup, IntTupHash> visited;
    std::vector<std::vector<IntTup>> layers(maxLightLevel + 1); // 0–16

    // Initialize top-level lights
    for (const auto& [pos, level] : lightSources) {
        if (visited.contains(pos)) continue;
        visited.insert(pos);
        layers[level].push_back(pos);
        setLightLevelFromOriginHere(pos, pos, level, lightmap);
    }

    std::shared_lock<std::shared_mutex> url;
    std::shared_lock<std::shared_mutex> nrl;


    if (!locked)
    {
        url = std::shared_lock<std::shared_mutex>(world->userDataMap->mutex());
        nrl = std::shared_lock<std::shared_mutex>(world->nonUserDataMap->mutex());
    }

    for (int level = maxLightLevel; level > 1; --level) {
        for (const auto& spot : layers[level]) {
            for (const auto& offset : neighbs) {
                IntTup neighbor = spot + offset;

                if (visited.contains(neighbor)) continue;
                if (std::find(transparents.begin(), transparents.end(), world->getLocked(neighbor)) == transparents.end()) continue;

                visited.insert(neighbor);
                layers[level - 1].push_back(neighbor);
                setLightLevelFromOriginHere(neighbor, spot, level - 1, lightmap);
            }
        }
    }
}

void lightPassOnChunk(World* world, TwoIntTup spot, int chunkw, int chunkh,
                      std::unordered_map<IntTup, LightSpot, IntTupHash>& lightmap, std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks, bool
                      locked)
{
    auto sources = getChunkLightSources(spot, world, chunkw, chunkh, lightmap, locked);
    propagateAllLightsLayered(world, sources, lightmap, implicatedChunks, locked);
}
