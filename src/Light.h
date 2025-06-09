//
// Created by jack on 4/12/2025.
//

#ifndef LIGHT_H
#define LIGHT_H

#include "Helpers.h"
#include "IntTup.h"
#include "PrecompHeader.h"
#include "world/World.h"

//struct Light
extern boost::sync_queue<TwoIntTup> lightOverlapNotificationQueue;
extern tbb::concurrent_hash_map<TwoIntTup, bool, TwoIntTupHashCompare> lightOverlapsQueued;


class ColorPack {
public:
    constexpr ColorPack() : packed(0) {}

    constexpr explicit ColorPack(uint8_t rgb) : packed(((rgb & 0x0F) << 8) | ((rgb & 0x0F) << 4) | (rgb & 0x0F)) {}

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
            std::clamp<int>((int)r() + (int)other.r(), 0, 15),
            std::clamp<int>((int)g() + (int)other.g(), 0, 15),
            std::clamp<int>((int)b() + (int)other.b(), 0, 15)
        );
    }

    constexpr ColorPack operator-(const ColorPack& other) const {
        return ColorPack(
            std::clamp<int>((int)r() - (int)other.r(), 0, 15),
            std::clamp<int>((int)g() - (int)other.g(), 0, 15),
            std::clamp<int>((int)b() - (int)other.b(), 0, 15)
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
            std::clamp<int>((int)r() - (int)value, 0, 15),
            std::clamp<int>((int)g() - (int)value, 0, 15),
            std::clamp<int>((int)b() - (int)value, 0, 15)
        );
    }

    constexpr ColorPack operator+(int value) const {
        return ColorPack(
            std::clamp<int>((int)r() + (int)value, 0, 15),
            std::clamp<int>((int)g() + (int)value, 0, 15),
            std::clamp<int>((int)b() + (int)value, 0, 15)
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







constexpr ColorPack SKYLIGHTVAL = ColorPack(15, 15, 15);
constexpr ColorPack TORCHLIGHTVAL = ColorPack(0, 0, 15);

struct LightRay
{
    uint16_t originhash = 0;
    ColorPack level;
};


struct LightSpot
{
    std::array<uint16_t, 16> originhashes = {};
    std::array<ColorPack, 16> colorpacks = {};
    uint8_t count : 5;
    void push_back(const LightRay& ray)
    {
        if (count < 16)
        {
            originhashes[count] = ray.originhash;
            colorpacks[count] = ray.level;
            count++;
        }
        //silently do nothing if not, no change necessary, brightness is at full .
    }
    ///Returns true if it deleted anything
    bool deleteRay(uint16_t originhash)
    {
        for (int i = 0; i < count; i++)
        {
            if (originhashes[i] == originhash)
            {
                //Shift everything after position i one position to the left
                for (int j = i; j < count - 1; j++)
                {
                    originhashes[j] = originhashes[j + 1];
                    colorpacks[j] = colorpacks[j + 1];
                }
                count--;
                return true; //Exit after finding and removing the first match
            }
        }
        return false;
    }

    ColorPack sum() {
        ColorPack res = {};
        for (int i = 0; i < count; ++i)
        {
            res += colorpacks[i];
        }
        return res;
    }
};


extern std::unique_ptr<std::vector<std::byte>> lmbuffer;
extern std::unique_ptr<boost::container::pmr::monotonic_buffer_resource> lmpool;

extern std::shared_ptr<std::vector<std::byte>> generatedChunksOnServerBuffer;
extern std::shared_ptr<boost::container::pmr::monotonic_buffer_resource> gcspool;


extern std::unique_ptr<std::vector<std::byte>> lmbufferouter;
extern std::unique_ptr<boost::container::pmr::monotonic_buffer_resource> lmpoolouter;


extern std::unique_ptr<std::vector<std::byte>> almbufferouter;
extern std::unique_ptr<boost::container::pmr::monotonic_buffer_resource> almpoolouter;

extern std::unique_ptr<std::vector<std::byte>> almbuffer;
extern std::unique_ptr<boost::container::pmr::monotonic_buffer_resource> almpool;

inline void initialize_buffers() {
    lmbuffer = std::make_unique<std::vector<std::byte>>((size_t)50 * 1024 * 1024);
    lmpool = std::make_unique<boost::container::pmr::monotonic_buffer_resource>(
        lmbuffer->data(), lmbuffer->size());

    almbuffer = std::make_unique<std::vector<std::byte>>((size_t)50 * 1024 * 1024);
    almpool = std::make_unique<boost::container::pmr::monotonic_buffer_resource>(
        almbuffer->data(), almbuffer->size());

    lmbufferouter = std::make_unique<std::vector<std::byte>>((size_t)50 * 1024 * 1024);
    lmpoolouter = std::make_unique<boost::container::pmr::monotonic_buffer_resource>(
        lmbufferouter->data(), lmbufferouter->size());

    almbufferouter = std::make_unique<std::vector<std::byte>>((size_t)50 * 1024 * 1024);
    almpoolouter = std::make_unique<boost::container::pmr::monotonic_buffer_resource>(
        almbufferouter->data(), almbufferouter->size());

    generatedChunksOnServerBuffer = std::make_shared<std::vector<std::byte>>((size_t)5 * 1024 * 1024);
    gcspool = std::make_shared<boost::container::pmr::monotonic_buffer_resource>(
        generatedChunksOnServerBuffer->data(), generatedChunksOnServerBuffer->size());
}


using InnerMapAlloc = boost::container::pmr::polymorphic_allocator<std::pair<const jl484_vec3, LightSpot>>;
using InnerMapType = boost::unordered_map<jl484_vec3, LightSpot, jl484_vec3_hash, std::equal_to<>, InnerMapAlloc>;



using OuterMapAlloc = boost::container::pmr::polymorphic_allocator<std::pair<const TwoIntTup, InnerMapType>>;
using OuterMapType = boost::unordered_map<TwoIntTup, InnerMapType, TwoIntTupHash, std::equal_to<>, OuterMapAlloc>;







class NewLightMapType
{

private:
    boost::container::pmr::memory_resource* outerresource;
    boost::container::pmr::memory_resource* resource;
public:
    OuterMapType lm;
    NewLightMapType() {}
    NewLightMapType(boost::container::pmr::memory_resource* res, boost::container::pmr::memory_resource* res2)
    : resource(res),  outerresource(res2), lm(OuterMapAlloc(outerresource)) {}

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

        auto [it, inserted] = lm.try_emplace(chunkPos, InnerMapType(InnerMapAlloc(resource)));
        it->second.insert_or_assign(loc, spot);
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
        auto& ls = lmentry.value();

        if (value != 0) {
            bool originfound = false;

            for (int i = 0; i < ls->count; i++) {
                const auto oh = ls->originhashes[i];
                if (oh == originhash) {
                    originfound = true;
                    if constexpr (ifHigher) {
                        if (ls->colorpacks[i] < value) {
                            ls->colorpacks[i] = value;
                        }
                    }
                    else {
                        ls->colorpacks[i] = value;
                    }
                    break;
                }
            }
            if (!originfound) {
                ls->push_back(LightRay{
                    .originhash = originhash, .level = value
                    });
            }
        }
        else {
            ls->deleteRay(originhash);
            if (ls->count == 0) {
                lightmap.erase(spot);
            }
        }
    }
    else if (value != 0) { // Only add new entry if value != 0
        auto lspot = LightSpot{};
        lspot.push_back(LightRay{
            .originhash = originhash, .level = value
            });
        lightmap.set(spot, lspot);
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
