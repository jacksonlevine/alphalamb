//
// Created by jack on 4/12/2025.
//

#include "Light.h"

#include "world/WorldRenderer.h"
boost::sync_queue<TwoIntTup> lightOverlapNotificationQueue = {};

void setLightLevelFromOriginHere(IntTup spot, IntTup origin, int value,
                                 LightMapType& lightmap)
{
    auto lmentry = lightmap.find(spot);
    bool originfound = false;
    auto originhash = IntTupHash{}(origin, true);
    if(lmentry != lightmap.end()) {
        if(value != 0) {
            for(auto ray : lmentry->second.rays) {
                if(ray.originhash == originhash) {
                    originfound = true;
                    ray.level = value;
                    break;
                }
            }
            if(!originfound) {
                lmentry->second.rays.push_back(LightRay{
                    .originhash = originhash, .level = value
                });
            }
        } else {
            lmentry->second.rays.erase(
                std::remove_if(
                    lmentry->second.rays.begin(),
                    lmentry->second.rays.end(),
                    [originhash](auto & ray) { return ray.originhash == originhash; }
                ),
                lmentry->second.rays.end()
            );
        }
    } else {
        lightmap.insert({spot, LightSpot{}});
        lightmap.at(spot).rays.push_back(LightRay{
            .originhash = originhash, .level = value
        });
    }
}

std::vector<std::pair<IntTup, int>> getChunkLightSources(const TwoIntTup& spot, World* world, int chunkw, int chunkh,
                                                         LightMapType& lightmap, bool locked)
{
    std::vector<std::pair<IntTup, int>> chunkLightSources;
    std::shared_lock<std::shared_mutex> url;
    std::shared_lock<std::shared_mutex> nrl;
    std::shared_lock<std::shared_mutex> lock3;

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
void propagateAllLightsLayered(World* world,
                              const std::vector<std::pair<IntTup, int>>& lightSources,
                              LightMapType& lightmap,
                              std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks,
                              bool locked) {
    constexpr int maxLightLevel = 16;
    constexpr int chunkWidth = 16, chunkHeight = 250, chunkDepth = 16;
    constexpr int pad = 16;
    constexpr size_t width = chunkWidth + 2 * pad;
    constexpr size_t height = chunkHeight + 2 * pad;
    constexpr size_t depth = chunkDepth + 2 * pad;
    constexpr size_t volume = width * height * depth;

    int minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
    if (!lightSources.empty()) {
        minX = maxX = lightSources[0].first.x;
        minY = maxY = lightSources[0].first.y;
        minZ = maxZ = lightSources[0].first.z;
        for (const auto& [pos, _] : lightSources) {
            minX = std::min(minX, pos.x);
            maxX = std::max(maxX, pos.x);
            minY = std::min(minY, pos.y);
            maxY = std::max(maxY, pos.y);
            minZ = std::min(minZ, pos.z);
            maxZ = std::max(maxZ, pos.z);
        }
        minX = (minX - (minX % 16)) - pad;
        maxX = (maxX - (maxX % 16)) + chunkWidth + pad - 1;
        minY = (minY - (minY % chunkHeight)) - pad;
        maxY = (maxY - (maxY % chunkHeight)) + chunkHeight + pad - 1;
        minZ = (minZ - (minZ % 16)) - pad;
        maxZ = (maxZ - (maxZ % 16)) + chunkDepth + pad - 1;
    }

    std::bitset<volume> visited;
    std::vector<std::vector<std::pair<IntTup, IntTup>>> layers(maxLightLevel + 1);
    std::array<bool, 256> transparentCache{};
    for (int i : transparents) {
        transparentCache[i] = true;
    }

    for (const auto& [pos, level] : lightSources) {
        size_t idx = (pos.x - minX) + (pos.z - minZ) * width + (pos.y - minY) * width * depth;
        if (idx >= volume) continue;
        if (visited[idx]) continue;
        visited[idx] = true;
        layers[level].push_back({pos, pos});
        setLightLevelFromOriginHere(pos, pos, level, lightmap);
        // if (implicatedChunks) {
        //     implicatedChunks->insert({pos.x / 16, pos.z / 16});
        // }
    }

    std::shared_lock<std::shared_mutex> url;
    std::shared_lock<std::shared_mutex> nrl;
    std::unique_lock<std::shared_mutex> lock3;
    if (!locked) {
        url = std::shared_lock<std::shared_mutex>(world->userDataMap->mutex());
        nrl = std::shared_lock<std::shared_mutex>(world->nonUserDataMap->mutex());
        lock3 = std::unique_lock<std::shared_mutex>(lightmapMutex);
    }

    for (int level = maxLightLevel; level > 1; --level) {
        for (const auto& [spot, source] : layers[level]) {
            for (const auto& offset : neighbs) {
                IntTup neighbor = spot + offset;
                size_t idx = (neighbor.x - minX) + (neighbor.z - minZ) * width + (neighbor.y - minY) * width * depth;
                if (idx >= volume) continue;
                if (visited[idx]) continue;
                if (!transparentCache[world->getLocked(neighbor)]) continue;

                visited[idx] = true;
                layers[level - 1].push_back({neighbor, source});
                setLightLevelFromOriginHere(neighbor, source, level - 1, lightmap);
                // if (implicatedChunks) {
                //     implicatedChunks->insert({neighbor.x / 16, neighbor.z / 16});
                // }
            }
        }
        layers[level].clear();
    }
}
void unpropagateAllLightsLayered(const std::vector<std::pair<IntTup, int>>& lightSourcesToRemove,
                                 LightMapType& lightmap,
                                 std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks, bool locked
) {
    constexpr int maxLightLevel = 16;
    constexpr int chunkWidth = 16, chunkHeight = 250, chunkDepth = 16;
    constexpr int pad = 16;
    constexpr size_t width = chunkWidth + 2 * pad;
    constexpr size_t height = chunkHeight + 2 * pad;
    constexpr size_t depth = chunkDepth + 2 * pad;
    constexpr size_t volume = width * height * depth;

    int minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
    if (!lightSourcesToRemove.empty()) {
        minX = maxX = lightSourcesToRemove[0].first.x;
        minY = maxY = lightSourcesToRemove[0].first.y;
        minZ = maxZ = lightSourcesToRemove[0].first.z;
        for (const auto& [pos, _] : lightSourcesToRemove) {
            minX = std::min(minX, pos.x);
            maxX = std::max(maxX, pos.x);
            minY = std::min(minY, pos.y);
            maxY = std::max(maxY, pos.y);
            minZ = std::min(minZ, pos.z);
            maxZ = std::max(maxZ, pos.z);
        }
        minX = (minX - (minX % 16)) - pad;
        maxX = (maxX - (maxX % 16)) + chunkWidth + pad - 1;
        minY = (minY - (minY % chunkHeight)) - pad;
        maxY = (maxY - (maxY % chunkHeight)) + chunkHeight + pad - 1;
        minZ = (minZ - (minZ % 16)) - pad;
        maxZ = (maxZ - (maxZ % 16)) + chunkDepth + pad - 1;
    }

    std::bitset<volume> visited;
    std::vector<std::vector<std::pair<IntTup, IntTup>>> layers(maxLightLevel + 1);



    std::unique_lock<std::shared_mutex> lock;
    if (!locked)
    {
        lock = std::unique_lock<std::shared_mutex>(lightmapMutex);
    }
    IntTupHash hasher;
    for (const auto& [pos, level] : lightSourcesToRemove) {
        size_t idx = (pos.x - minX) + (pos.z - minZ) * width + (pos.y - minY) * width * depth;
        if (idx >= volume) continue;
        if (visited[idx]) continue;
        visited[idx] = true;
        layers[level].push_back({pos, pos});
        setLightLevelFromOriginHere(pos, pos, 0, lightmap);
        // if (implicatedChunks) {
        //     implicatedChunks->insert({pos.x / 16, pos.z / 16});
        // }
    }

    for (int level = maxLightLevel; level > 1; --level) {
        for (const auto& [spot, origin] : layers[level]) {
            auto originhash = hasher(origin, true);
            for (const auto& offset : neighbs) {
                IntTup neighbor = spot + offset;
                size_t idx = (neighbor.x - minX) + (neighbor.z - minZ) * width + (neighbor.y - minY) * width * depth;
                if (idx >= volume) continue;
                if (visited[idx]) continue;
                std::cout << "spot: " << neighbor.x << " " << neighbor.y << " " << neighbor.z  << std::endl;

                auto lmentry = lightmap.find(neighbor);
                if (lmentry != lightmap.end()) {
                    auto& rays = lmentry->second.rays;
                    for (auto it = rays.begin(); it != rays.end();) {
                        if (it->originhash == originhash) {
                            it = rays.erase(it);
                        } else {
                            ++it;
                        }
                    }
                    if (rays.empty()) {
                        lightmap.erase(lmentry);
                    } else {
                        visited[idx] = true;
                        layers[level - 1].push_back({neighbor, origin});
                        setLightLevelFromOriginHere(neighbor, origin, 0, lightmap);
                        // if (implicatedChunks) {
                        //     implicatedChunks->insert({neighbor.x / 16, neighbor.z / 16});
                        // }
                    }
                }
            }
        }
        layers[level].clear();
    }
}
void lightPassOnChunk(World* world, TwoIntTup spot, int chunkw, int chunkh,
                      LightMapType& lightmap, bool
                      locked)
{
    auto implicatedChunks = std::unordered_set<TwoIntTup, TwoIntTupHash>();

    auto sources = getChunkLightSources(spot, world, chunkw, chunkh, lightmap, locked);
    unpropagateAllLightsLayered(sources, lightmap, &implicatedChunks, locked);
    propagateAllLightsLayered(world, sources, lightmap, &implicatedChunks, locked);

    for (const auto & spot : implicatedChunks)
    {
        lightOverlapNotificationQueue.push(spot);
    }
}
