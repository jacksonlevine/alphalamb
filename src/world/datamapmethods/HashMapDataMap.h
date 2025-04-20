//
// Created by jack on 1/27/2025.
//
#ifndef HASHMAPUSERDATAMAP_H
#define HASHMAPUSERDATAMAP_H
#include "../DataMap.h"
//#define MEASURE_LOOKUP 1
///An unordered_map with a shared_mutex under the hood to be thread safe.
/// Also has an Unsafe api for bypassing the mutex, but always lock the mutex yourself in these cases.
/// Use getUniqueLock() and std::shared_lock<std::shared_mutex>(mapmutex) for writes and reads, respectively.
class HashMapDataMap : public DataMap {
public:
    void erase(const IntTup& spot, bool locked = false) override;

    void erase(const TwoIntTup& spot, bool locked = false) override;
    std::unique_lock<std::shared_mutex> getUniqueLock() override;
    std::unique_ptr<Iterator> createIterator() override;
    std::shared_mutex& mutex() override;
    std::optional<BlockType> get(const IntTup& spot) const override;
    std::optional<BlockType> getUnsafe(const IntTup& spot) const override;
    void clear() override;
    void set(const IntTup& spot, BlockType block) override;
    void setUnsafe(const IntTup& spot, BlockType block) override;
    mutable std::shared_mutex mapmutex = {};
private:
    friend class HashMapDataMapIterator;

    // Calculate chunk coordinates from world coordinates (only in X and Z)
    static TwoIntTup getChunkPos(const IntTup& worldPos) {
        return TwoIntTup(std::floor(worldPos.x / 16.0f), std::floor(worldPos.z / 16.0f));
    }

    // Map of chunks: key is chunk position (x,z), value is a map of positions (with full x,y,z) to blocks
    std::unordered_map<TwoIntTup, std::unordered_map<IntTup, BlockType, IntTupHash>, TwoIntTupHash> chunks;

#ifdef MEASURE_LOOKUP
    mutable size_t lookup_count = 0;
    mutable std::chrono::duration<double> cumulative_lookup_time = std::chrono::duration<double>::zero();
#endif
};

class HashMapDataMapIterator final : public DataMap::Iterator
{
public:
    explicit HashMapDataMapIterator(HashMapDataMap& dataMap);
    bool hasNext() const override;
    std::pair<const IntTup&, BlockType&> next() override;

private:
    HashMapDataMap& dataMap;
    using ChunkMap = std::unordered_map<TwoIntTup, std::unordered_map<IntTup, BlockType, IntTupHash>, TwoIntTupHash>;
    using BlockMap = std::unordered_map<IntTup, BlockType, IntTupHash>;

    typename ChunkMap::iterator chunkIt;
    typename ChunkMap::iterator chunkEnd;
    typename BlockMap::iterator blockIt;
    typename BlockMap::iterator blockEnd;

    mutable std::pair<IntTup, BlockType> currentPair;

    // Advances to the next valid block or chunk
    void advanceToNext();
};
#endif // HASHMAPUSERDATAMAP_H