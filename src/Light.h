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

extern boost::sync_queue<TwoIntTup> lightOverlapNotificationQueue;

constexpr ColorPack SKYLIGHTVAL = ColorPack(12, 12, 12);
constexpr ColorPack TORCHLIGHTVAL = ColorPack(0, 0, 10);

struct LightRay
{
    uint32_t originhash;
    ColorPack level;
};

class LightSpot {
public:
    boost::container::vector<LightRay> rays = {};

    ColorPack sum() {
        ColorPack res = {};
        for (auto ray : rays) {
            res += ray.level;
        }
        return res;
    }
    LightSpot()
    {
        rays.reserve(5);
    }
    ~LightSpot() = default;
    LightSpot(const LightSpot& other) = delete;
    LightSpot& operator=(const LightSpot& other) = delete;
    LightSpot& operator=(LightSpot&& other)
    {
        rays = std::move(other.rays);
        return *this;
    };
    LightSpot(LightSpot&& other)
    {
        rays = std::move(other.rays);
    }
};

// Memory-efficient LightMap class with same API as original
class LightMapType {
private:
    struct ChunkData {
        boost::unordered_map<IntTup, LightSpot, IntTupHash> spots;
    };

    int chunkSize;
    boost::unordered_map<TwoIntTup, ChunkData, TwoIntTupHash> chunks;

    TwoIntTup getChunkPos(const IntTup& pos) const {
        return TwoIntTup(
            std::floor(static_cast<float>(pos.x) / chunkSize),
            std::floor(static_cast<float>(pos.z) / chunkSize)
        );
    }

    IntTup getLocalPos(const IntTup& pos, const TwoIntTup& chunkPos) const {
        return IntTup(
            pos.x - std::floor(chunkPos.x * chunkSize),
            pos.y,
            pos.z - std::floor(chunkPos.z * chunkSize)
        );
    }

public:
    // Iterator class - provides same interface as original map iterator
    class iterator {
    private:
        friend class LightMapType;
        LightMapType* map;
        bool isEnd;
        TwoIntTup currentChunk;
        typename boost::unordered_map<IntTup, LightSpot, IntTupHash>::iterator spotIt;
        mutable std::pair<IntTup, LightSpot> current;

        iterator(LightMapType* map, bool end)
            : map(map), isEnd(end), currentChunk(), spotIt() {}

        iterator(LightMapType* map, const TwoIntTup& chunk,
            typename boost::unordered_map<IntTup, LightSpot, IntTupHash>::iterator spot)
            : map(map), isEnd(false), currentChunk(chunk), spotIt(spot) {}

    public:
        std::pair<const IntTup, LightSpot>* operator->() {
            if (isEnd) return nullptr;
            // Convert local position to world position
            IntTup worldPos(
                spotIt->first.x - currentChunk.x * map->chunkSize,
                spotIt->first.y,
                spotIt->first.z - currentChunk.z * map->chunkSize
            );
            current.first = worldPos;
            current.second = std::move(spotIt->second);
            return reinterpret_cast<std::pair<const IntTup, LightSpot>*>(&current);
        }

        bool operator==(const iterator& other) const {
            return (isEnd && other.isEnd) ||
                (!isEnd && !other.isEnd && currentChunk == other.currentChunk && spotIt == other.spotIt);
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };

    LightMapType(int chunkSize = 16) : chunkSize(chunkSize) {}

    LightSpot& operator[](const IntTup& pos) {
        TwoIntTup chunkPos = getChunkPos(pos);
        IntTup localPos = getLocalPos(pos, chunkPos);
        return chunks[chunkPos].spots[localPos];
    }

    iterator find(const IntTup& pos) {
        TwoIntTup chunkPos = getChunkPos(pos);
        IntTup localPos = getLocalPos(pos, chunkPos);

        auto chunkIt = chunks.find(chunkPos);
        if (chunkIt == chunks.end()) {
            return end();
        }

        auto spotIt = chunkIt->second.spots.find(localPos);
        if (spotIt == chunkIt->second.spots.end()) {
            return end();
        }

        return iterator(this, chunkPos, spotIt);
    }

    iterator end() {
        return iterator(this, true);
    }

    void erase(iterator it) {
        if (it.isEnd) return;

        auto& chunkData = chunks[it.currentChunk];
        chunkData.spots.erase(it.spotIt);

        // Clean up empty chunks to save memory
        if (chunkData.spots.empty()) {
            chunks.erase(it.currentChunk);
        }
    }

    bool contains(const IntTup& pos) const {
        TwoIntTup chunkPos = getChunkPos(pos);
        IntTup localPos = getLocalPos(pos, chunkPos);

        auto chunkIt = chunks.find(chunkPos);
        if (chunkIt == chunks.end()) {
            return false;
        }

        return chunkIt->second.spots.contains(localPos);
    }

    // Access the light spot at a position, throws if not found
    LightSpot& at(const IntTup& pos) {
        TwoIntTup chunkPos = getChunkPos(pos);
        IntTup localPos = getLocalPos(pos, chunkPos);

        auto chunkIt = chunks.find(chunkPos);
        if (chunkIt == chunks.end()) {
            throw std::out_of_range("No chunk found for position");
        }

        auto spotIt = chunkIt->second.spots.find(localPos);
        if (spotIt == chunkIt->second.spots.end()) {
            throw std::out_of_range("No light spot found at position");
        }

        return spotIt->second;
    }

    void eraseChunk(const TwoIntTup& chunkPos) {
        chunks.erase(chunkPos);
    }

    // Const version of at
    const LightSpot& at(const IntTup& pos) const {
        TwoIntTup chunkPos = getChunkPos(pos);
        IntTup localPos = getLocalPos(pos, chunkPos);

        auto chunkIt = chunks.find(chunkPos);
        if (chunkIt == chunks.end()) {
            throw std::out_of_range("No chunk found for position");
        }

        auto spotIt = chunkIt->second.spots.find(localPos);
        if (spotIt == chunkIt->second.spots.end()) {
            throw std::out_of_range("No light spot found at position");
        }

        return spotIt->second;
    }
};

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
    auto originhash = IntTupHash{}(origin, true);
    if (value != 0) {
        // Add or update ray at spot
        bool originfound = false;
        if (lightmap.contains(spot)) {
            auto& rays = lightmap.at(spot).rays;
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
        }
        if (!originfound) {
            lightmap[spot].rays.push_back(LightRay{
                .originhash = originhash, .level = value
                });
        }
    }
    else {
        // Remove ray at spot
        if (lightmap.contains(spot)) {
            auto& rays = lightmap.at(spot).rays;
            rays.erase(
                std::remove_if(
                    rays.begin(),
                    rays.end(),
                    [originhash](const auto& ray) { return ray.originhash == originhash; }
                ),
                rays.end()
            );
            if (rays.empty()) {
                auto it = lightmap.find(spot);
                if (it != lightmap.end()) {
                    lightmap.erase(std::move(it));
                }
            }
        }
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