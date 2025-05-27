//
// Created by jack on 4/12/2025.
//

#ifndef LIGHT_H
#define LIGHT_H

#include "Helpers.h"
#include "IntTup.h"
#include "PrecompHeader.h"
#include "world/World.h"





class ColorPack {
public:
    constexpr ColorPack() : packed(0) {}

    constexpr ColorPack(uint8_t r, uint8_t g, uint8_t b)
        : packed(((r & 0x0F) << 8) | ((g & 0x0F) << 4) | (b & 0x0F)) {}

    constexpr explicit ColorPack(uint16_t packedColor)
        : packed(packedColor & 0x0FFF) {}

    constexpr uint8_t r() const { return (packed >> 8) & 0x0F; }
    constexpr uint8_t g() const { return (packed >> 4) & 0x0F; }
    constexpr uint8_t b() const { return packed & 0x0F; }

    constexpr void set(uint8_t r, uint8_t g, uint8_t b) {
        packed = ((r & 0x0F) << 8) | ((g & 0x0F) << 4) | (b & 0x0F);
    }

    constexpr operator uint16_t() const {
        return packed;
    }

    constexpr ColorPack& operator=(uint16_t value) {
        packed = value & 0x0FFF;
        return *this;
    }

    constexpr ColorPack operator+(const ColorPack& other) const {
        return ColorPack(
            std::clamp<int>(r() + other.r(), 0, 15),
            std::clamp<int>(g() + other.g(), 0, 15),
            std::clamp<int>(b() + other.b(), 0, 15)
        );
    }

    constexpr ColorPack operator-(const ColorPack& other) const {
        return ColorPack(
            std::clamp<int>(r() - other.r(), 0, 15),
            std::clamp<int>(g() - other.g(), 0, 15),
            std::clamp<int>(b() - other.b(), 0, 15)
        );
    }

    constexpr ColorPack& operator+=(const ColorPack& other) {
        *this = *this + other;
        return *this;
    }

    constexpr ColorPack& operator-=(const ColorPack& other) {
        *this = *this - other;
        return *this;
    }

    constexpr ColorPack operator-(int value) const {
        return ColorPack(
            std::clamp<int>(r() - value, 0, 15),
            std::clamp<int>(g() - value, 0, 15),
            std::clamp<int>(b() - value, 0, 15)
        );
    }

    constexpr ColorPack operator+(int value) const {
        return ColorPack(
            std::clamp<int>(r() + value, 0, 15),
            std::clamp<int>(g() + value, 0, 15),
            std::clamp<int>(b() + value, 0, 15)
        );
    }

    constexpr ColorPack& operator-=(int value) {
        *this = *this - value;
        return *this;
    }

    constexpr ColorPack& operator+=(int value) {
        *this = *this + value;
        return *this;
    }

private:
    uint16_t packed;
};




//struct Light
extern boost::sync_queue<TwoIntTup> lightOverlapNotificationQueue;


constexpr ColorPack SKYLIGHTVAL = ColorPack(15, 15, 15);
constexpr ColorPack TORCHLIGHTVAL = ColorPack(0, 0, 15);

struct LightRay
{
    uint16_t originhash;
    ColorPack level;
};

using LightRayPoolAlloc = boost::pool_allocator<
    LightRay,
    boost::default_user_allocator_new_delete,
    boost::details::pool::default_mutex,
    64
>;

struct LightSpot {
    boost::container::vector<LightRay> rays;

    ColorPack sum() {
        ColorPack res = {};
        for (auto ray : rays) {
            res += ray.level;
        }
        return res;
    }
};


using InnerPoolAlloc = boost::pool_allocator<
    std::pair<const jl484_vec3, LightSpot>,
    boost::default_user_allocator_new_delete,
    boost::details::pool::default_mutex,
    256,
    0
>;

using InnerMapType = boost::unordered_map<jl484_vec3, LightSpot, jl484_vec3_hash, std::equal_to<>, InnerPoolAlloc>;

using OuterPoolAlloc = boost::pool_allocator<
    std::pair<const TwoIntTup, InnerMapType>,
    boost::default_user_allocator_new_delete,
    boost::details::pool::default_mutex,
    64,
    0
>;

using OuterMapType = boost::unordered_map<TwoIntTup, InnerMapType, TwoIntTupHash, std::equal_to<>, OuterPoolAlloc>;

class NewLightMapType
{

public:
    OuterMapType lm;
    std::optional<LightSpot*> get(const IntTup& t)
    {
        auto loc = worldToChunkLocalPos(t);
        auto chunkPos = world3ToChunkPos(t);

        auto outerIt = lm.find(chunkPos);
        if (outerIt != lm.end())
        {
            auto innerIt = outerIt->second.find(loc);
            if (innerIt != outerIt->second.end())
            {
                return &innerIt->second;
            }
        }
        return std::nullopt;
    }

    void set(const IntTup& t, const LightSpot& spot)
    {
        auto loc = worldToChunkLocalPos(t);
        auto chunkPos = world3ToChunkPos(t);

        lm[chunkPos].insert_or_assign(loc, spot);
    }

    void erase(const IntTup& t)
    {
        auto loc = worldToChunkLocalPos(t);
        auto chunkPos = world3ToChunkPos(t);

        auto outerIt = lm.find(chunkPos);
        if (outerIt != lm.end())
        {
            outerIt->second.erase(loc);
            // Optionally clean up empty chunks
            if (outerIt->second.empty())
            {
                lm.erase(outerIt);
            }
        }
    }

    void deleteChunk(const TwoIntTup& c)
    {
        lm.erase(c);
    }

    // Useful for debugging memory usage
    size_t getChunkCount() const { return lm.size(); }
    size_t getTotalLightCount() const
    {
        size_t total = 0;
        for (const auto& [chunk, innerMap] : lm)
        {
            total += innerMap.size();
        }
        return total;
    }
};
using LightMapType = NewLightMapType;

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
void setLightLevelFromOriginHere(IntTup spot, IntTup origin, ColorPack value,
    LightMapType& lightmap)
{
    auto lmentry = lightmap.get(spot);
    auto originhash = IntTupHash{}(origin, true);
    if (lmentry != std::nullopt) {
        auto& rays = lmentry.value()->rays;

        if (value != 0) {
            bool originfound = false;

            for (auto& ray : rays) {
                if (ray.originhash == originhash) {
                    originfound = true;
                    if constexpr (ifHigher) {
                        if (ray.level < value) {
                            ray.level = value;
                        }
                    }
                    else {
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
        }
        else {
            rays.erase(
                std::remove_if(
                    rays.begin(),
                    rays.end(),
                    [originhash](const auto& ray) { return ray.originhash == originhash; }
                ),
                rays.end()
            );
            if (rays.empty()) {
                lightmap.erase(spot);
            }
        }
    }
    else if (value != 0) { // Only add new entry if value != 0
        lightmap.set(spot, LightSpot{});
        auto & vec = lightmap.get(spot).value()->rays;
        vec.push_back(LightRay{
            .originhash = originhash, .level = value
            });
    }
}

struct ChunkLightSources
{
    std::vector<std::pair<IntTup, ColorPack>> oldBlockSources;
    std::vector<std::pair<IntTup, ColorPack>> newBlockSources;
    std::vector<std::pair<IntTup, ColorPack>> ambientOldSources;
    std::vector<std::pair<IntTup, ColorPack>> ambientNewSources;
};


ChunkLightSources getChunkLightSourcesBlockAndAmbient(
    const TwoIntTup& spot, World* world, int chunkw, int chunkh, bool
    locked);

void propagateAllLightsLayered(World* world, const std::vector<std::pair<IntTup, ColorPack>>& lightSources,
    LightMapType& lightmap, TwoIntTup chunkOrigin,
    std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks = nullptr, bool
    locked = false);


void unpropagateAllLightsLayered(const std::vector<std::pair<IntTup, ColorPack>>& lightSourcesToRemove, LightMapType&
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
