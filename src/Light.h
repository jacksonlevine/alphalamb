//
// Created by jack on 4/12/2025.
//

#ifndef LIGHT_H
#define LIGHT_H

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


constexpr ColorPack SKYLIGHTVAL = ColorPack(12,12,12);
constexpr ColorPack TORCHLIGHTVAL = ColorPack(0,0,10);

struct LightRay
{
    uint32_t originhash;
    ColorPack level;
};

struct LightSpot {
    boost::container::vector<LightRay> rays;

    ColorPack sum() {
        ColorPack res = {};
        for(auto ray : rays) {
            res += ray.level;
        }
        return res;
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
void setLightLevelFromOriginHere(IntTup spot, IntTup origin, ColorPack value,
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
