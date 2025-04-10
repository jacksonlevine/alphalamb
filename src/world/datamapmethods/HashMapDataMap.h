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
    void erase(const IntTup& spot, bool locked = true) override;

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
    std::unordered_map<IntTup, BlockType, IntTupHash> map;

#ifdef MEASURE_LOOKUP
    mutable size_t lookup_count = 0;
    mutable std::chrono::duration<double> cumulative_lookup_time = std::chrono::duration<double>::zero();
#endif
};

class HashMapDataMapIterator final : public DataMap::Iterator
{
public:
    explicit HashMapDataMapIterator(HashMapDataMap& dataMap)
        : it(dataMap.map.begin()), end(dataMap.map.end()) {}

    bool hasNext() const override {
        return it != end;
    }

    std::pair<const IntTup&, BlockType&> next() override {
        if (it == end) throw std::out_of_range("Iterator out of range");
        return *(it++);
    }

private:
    std::unordered_map<IntTup, BlockType, IntTupHash>::iterator it, end;
};

#endif // HASHMAPUSERDATAMAP_H