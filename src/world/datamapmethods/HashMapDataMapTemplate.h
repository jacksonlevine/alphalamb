// HashMapDataMapTemplate.h
#ifndef HASHMAPDATAMAPTEMPLATE_H
#define HASHMAPDATAMAPTEMPLATE_H

#include "../../PrecompHeader.h"
#include "../../Helpers.h"
#include "../jl484_vec3.h"
template <typename ValueType>
class HashMapDataMapTemplate {
public:
    void erase(const IntTup& spot, bool locked = false);
    void erase(const TwoIntTup& spot, bool locked = false);
    std::unique_lock<std::shared_mutex> getUniqueLock();
    std::shared_mutex& mutex();
    std::optional<BlockType> get(const IntTup& spot) const;
    std::optional<BlockType> getUnsafe(const IntTup& spot) const;
    void clear();
    void set(const IntTup& spot, BlockType value);
    void setUnsafe(const IntTup& spot, BlockType value);

    // Iterator class that returns world positions
    class Iterator {
    public:
        explicit Iterator(HashMapDataMapTemplate<ValueType>& dataMap);
        bool hasNext() const;
        std::pair<IntTup, ValueType&> next();

    private:
        HashMapDataMapTemplate<ValueType>& dataMap;

        using BlockMap = boost::unordered_map<jl484_vec3, ValueType, jl484_vec3_hash>;
        using ChunkMap = boost::unordered_map<TwoIntTup, BlockMap, TwoIntTupHash>;

        typename ChunkMap::iterator chunkIt;
        typename ChunkMap::iterator chunkEnd;
        typename BlockMap::iterator blockIt;
        typename BlockMap::iterator blockEnd;

        // Convert local position to world position
        IntTup localToWorldPos(const TwoIntTup& chunkPos, const jl484_vec3& localPos) const;
        void advanceToNext();
    };

    std::unique_ptr<Iterator> createIterator() {
        return std::make_unique<Iterator>(*this);
    }

private:
    // Map of chunks: key is chunk position (x,z), value is a map of positions (with full x,y,z) to values
    boost::unordered_map<TwoIntTup, boost::unordered_map<jl484_vec3, ValueType, jl484_vec3_hash>, TwoIntTupHash> chunks;
    mutable std::shared_mutex mapmutex;

#ifdef MEASURE_LOOKUP
    mutable size_t lookup_count = 0;
    mutable std::chrono::duration<double> cumulative_lookup_time = std::chrono::duration<double>::zero();
#endif
};

// Template method implementations

template <typename ValueType>
void HashMapDataMapTemplate<ValueType>::erase(const IntTup& spot, bool locked) {
    auto chunkPos = world3ToChunkPos(spot);
    auto chunkLocalPos = worldToChunkLocalPos(spot);
    if (!locked) {
        std::unique_lock<std::shared_mutex> lock(mapmutex);
        if (chunks.find(chunkPos) != chunks.end()) {
            chunks[chunkPos].erase(chunkLocalPos);
            // Remove empty chunks
            if (chunks[chunkPos].empty()) {
                chunks.erase(chunkPos);
            }
        }
    } else {
        if (chunks.find(chunkPos) != chunks.end()) {
            chunks[chunkPos].erase(chunkLocalPos);
            // Remove empty chunks
            if (chunks[chunkPos].empty()) {
                chunks.erase(chunkPos);
            }
        }
    }
}

template <typename ValueType>
void HashMapDataMapTemplate<ValueType>::erase(const TwoIntTup& spot, bool locked) {
    auto chunkPos = spot;

    if (!locked) {
        std::unique_lock<std::shared_mutex> lock(mapmutex);
        if (chunks.find(chunkPos) != chunks.end()) {
            chunks.erase(chunkPos);
        }
    } else {
        if (chunks.find(chunkPos) != chunks.end()) {
            chunks.erase(chunkPos);
        }
    }
}

template <typename ValueType>
std::unique_lock<std::shared_mutex> HashMapDataMapTemplate<ValueType>::getUniqueLock() {
    return std::unique_lock<std::shared_mutex>(mutex());
}

template <typename ValueType>
std::shared_mutex& HashMapDataMapTemplate<ValueType>::mutex() {
    return this->mapmutex;
}

template <typename ValueType>
std::optional<BlockType> HashMapDataMapTemplate<ValueType>::get(const IntTup& spot) const {
#ifdef MEASURE_LOOKUP
    auto start = std::chrono::high_resolution_clock::now();
#endif
    std::shared_lock<std::shared_mutex> lock(mapmutex);

    auto chunkPos = world3ToChunkPos(spot);
    auto chunkLocalSpot = worldToChunkLocalPos(spot);

    std::optional<BlockType> value = std::nullopt;
    auto chunkIt = chunks.find(chunkPos);
    if (chunkIt != chunks.end()) {
        auto blockIt = chunkIt->second.find(chunkLocalSpot);
        if (blockIt != chunkIt->second.end()) {
            value = 0u | blockIt->second;
        }
    }

#ifdef MEASURE_LOOKUP
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cumulative_lookup_time += elapsed;
    lookup_count++;
    if (lookup_count % 1000 == 0) {
        double average_lookup_time = cumulative_lookup_time.count() / lookup_count;
    }
#endif
    return value;
}

template <typename ValueType>
std::optional<BlockType> HashMapDataMapTemplate<ValueType>::getUnsafe(const IntTup& spot) const {
    auto chunkPos = world3ToChunkPos(spot);
    auto chunkLocalSpot = worldToChunkLocalPos(spot);
    std::optional<BlockType> value = std::nullopt;
    auto chunkIt = chunks.find(chunkPos);
    if (chunkIt != chunks.end()) {
        auto blockIt = chunkIt->second.find(chunkLocalSpot);
        if (blockIt != chunkIt->second.end()) {
            value = 0u | blockIt->second;
        }
    }

    return value;
}

template <typename ValueType>
void HashMapDataMapTemplate<ValueType>::clear() {
    std::unique_lock<std::shared_mutex> lock(mapmutex);
    chunks.clear();
}

template <typename ValueType>
void HashMapDataMapTemplate<ValueType>::set(const IntTup& spot, BlockType value) {
    if (spot.y > 255 || spot.y < 0) return;
    std::unique_lock<std::shared_mutex> lock(mapmutex);
    auto chunkPos = world3ToChunkPos(spot);
    auto chunkLocalSpot = worldToChunkLocalPos(spot);
    chunks[chunkPos][chunkLocalSpot] = static_cast<ValueType>(value);
}

template <typename ValueType>
void HashMapDataMapTemplate<ValueType>::setUnsafe(const IntTup& spot, BlockType value) {
    if (spot.y > 255 || spot.y < 0) return;
    auto chunkPos = world3ToChunkPos(spot);
    auto chunkLocalSpot = worldToChunkLocalPos(spot);
    chunks[chunkPos][chunkLocalSpot] = static_cast<ValueType>(value);
}

// Iterator implementations

template <typename ValueType>
HashMapDataMapTemplate<ValueType>::Iterator::Iterator(HashMapDataMapTemplate<ValueType>& dataMap)
    : dataMap(dataMap),
      chunkIt(dataMap.chunks.begin()),
      chunkEnd(dataMap.chunks.end()) {
    if (chunkIt != chunkEnd) {
        blockIt = chunkIt->second.begin();
        blockEnd = chunkIt->second.end();
        advanceToNext();
    }
}

template <typename ValueType>
bool HashMapDataMapTemplate<ValueType>::Iterator::hasNext() const {
    return chunkIt != chunkEnd;
}

template <typename ValueType>
std::pair<IntTup, ValueType&> HashMapDataMapTemplate<ValueType>::Iterator::next() {
    if (!hasNext()) {std::cout << "advanced iterator past end" <<std::endl;}

    // Convert local position to world position
    IntTup worldPos = localToWorldPos(chunkIt->first, blockIt->first);

    // Create pair with world position and block value
    auto result = std::make_pair(worldPos, std::ref(blockIt->second));

    // Move to next element
    ++blockIt;
    advanceToNext();

    return result;
}

template <typename ValueType>
void HashMapDataMapTemplate<ValueType>::Iterator::advanceToNext() {
    // If we've reached the end of blocks in the current chunk
    while (chunkIt != chunkEnd && blockIt == blockEnd) {
        ++chunkIt;
        if (chunkIt != chunkEnd) {
            blockIt = chunkIt->second.begin();
            blockEnd = chunkIt->second.end();
        }
    }
}

template <typename ValueType>
IntTup HashMapDataMapTemplate<ValueType>::Iterator::localToWorldPos(const TwoIntTup& chunkPos, const jl484_vec3& localPos) const {
    // Convert chunk-local coordinates to world coordinates
    // Formula: chunkPos.x * 16 + localPos.x, localPos.y, chunkPos.z * 16 + localPos.z
    return IntTup(
        chunkPos.x * 16 + static_cast<int>(localPos.x),
        localPos.y,
        chunkPos.z * 16 + static_cast<int>(localPos.z)
    );
}

#endif // HASHMAPDATAMAPTEMPLATE_H