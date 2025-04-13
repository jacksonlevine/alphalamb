//
// Created by jack on 4/12/2025.
//

#include "Light.h"

void setLightLevelFromOriginHere(IntTup spot, IntTup origin, int value,
    std::unordered_map<IntTup, LightSpot, IntTupHash>& lightmap)
{
    auto lmentry = lightmap.find(spot);
    bool originfound = false;
    if(lmentry != lightmap.end()) {
        if(value != 0) {
            for(auto ray : lmentry->second.rays) {
                if(ray.origin == origin) {
                    originfound = true;
                    ray.level = value;
                    break;
                }
            }
            if(!originfound) {
                lmentry->second.rays.push_back(LightRay{
                    .origin = origin, .level = value
                });
            }
        } else {
            std::erase_if(lmentry->second.rays, [origin](auto & ray) { return ray.origin == origin; });
        }
    } else {
        lightmap.insert({spot, LightSpot{}});
        lightmap.at(spot).rays.push_back(LightRay{
            .origin = origin, .level = value
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
                               std::unordered_map<IntTup, LightSpot, IntTupHash>& lightmap,
                               std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks,
                               bool locked)
{
    constexpr int maxLightLevel = 16;

    std::unordered_set<IntTup, IntTupHash> visited;
    std::vector<std::vector<std::pair<IntTup, IntTup>>> layers(maxLightLevel + 1); // (position, source)

    // Initialize top-level lights
    for (const auto& [pos, level] : lightSources) {
        if (visited.contains(pos)) continue;
        visited.insert(pos);
        layers[level].push_back({pos, pos});
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
        for (const auto& [spot, source] : layers[level]) {
            for (const auto& offset : neighbs) {
                IntTup neighbor = spot + offset;

                if (visited.contains(neighbor)) continue;
                if (std::find(transparents.begin(), transparents.end(), world->getLocked(neighbor)) == transparents.end()) continue;

                visited.insert(neighbor);
                layers[level - 1].push_back({neighbor, source});
                setLightLevelFromOriginHere(neighbor, source, level - 1, lightmap);
            }
        }
    }
}


void unpropagateAllLightsLayered(const std::vector<std::pair<IntTup, int>>& lightSourcesToRemove,
                                std::unordered_map<IntTup, LightSpot, IntTupHash>& lightmap,
                                std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks,
                                bool locked)
{
    constexpr int maxLightLevel = 16;

    std::unordered_set<IntTup, IntTupHash> visited;
    std::vector<std::vector<std::pair<IntTup, IntTup>>> layers(maxLightLevel + 1); // Position and its source origin

    // Initialize top-level lights to remove
    for (const auto& [pos, level] : lightSourcesToRemove) {
        if (visited.contains(pos)) continue;
        visited.insert(pos);
        layers[level].push_back({pos, pos}); // Source is its own origin

        // Remove light from origin point
        setLightLevelFromOriginHere(pos, pos, 0, lightmap);
    }

    // Process each layer, from brightest to dimmest
    for (int level = maxLightLevel; level > 1; --level) {
        for (const auto& [spot, origin] : layers[level]) {
            for (const auto& offset : neighbs) {
                IntTup neighbor = spot + offset;

                if (visited.contains(neighbor)) continue;

                // Check if this neighbor was affected by the light we're removing
                auto lmentry = lightmap.find(neighbor);
                if (lmentry != lightmap.end()) {
                    bool originFound = false;

                    // Check if this spot has a ray from the ORIGINAL source (not the neighbor it came from)
                    for (const auto& ray : lmentry->second.rays) {
                        if (ray.origin == origin) { // Check against the original light source
                            originFound = true;
                            break;
                        }
                    }

                    if (originFound) {
                        visited.insert(neighbor);
                        layers[level - 1].push_back({neighbor, origin});
                        setLightLevelFromOriginHere(neighbor, origin, 0, lightmap);
                    }
                }
            }
        }
    }
}

void lightPassOnChunk(World* world, TwoIntTup spot, int chunkw, int chunkh,
                      std::unordered_map<IntTup, LightSpot, IntTupHash>& lightmap, std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks, bool
                      locked)
{
    auto sources = getChunkLightSources(spot, world, chunkw, chunkh, lightmap, locked);
    unpropagateAllLightsLayered(sources, lightmap, implicatedChunks, locked);
    propagateAllLightsLayered(world, sources, lightmap, implicatedChunks, locked);
}
