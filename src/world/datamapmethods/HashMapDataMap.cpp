#include "HashMapDataMap.h"

#include "../../BlockType.h"

void HashMapDataMap::erase(const IntTup& spot, bool locked)
{
    if (!locked)
    {
        std::unique_lock<std::shared_mutex> lock(mapmutex);
        this->map.erase(spot);
    } else
    {
        this->map.erase(spot);
    }
}

std::unique_lock<std::shared_mutex> HashMapDataMap::getUniqueLock()
{
    return std::unique_lock<std::shared_mutex>(mutex());
}

std::unique_ptr<DataMap::Iterator> HashMapDataMap::createIterator()
{
    return std::make_unique<HashMapDataMapIterator>(*this);
}

std::shared_mutex& HashMapDataMap::mutex()
{
    return this->mapmutex;
}

std::optional<BlockType> HashMapDataMap::get(const IntTup& spot) const {

#ifdef MEASURE_LOOKUP
    auto start = std::chrono::high_resolution_clock::now();
#endif

    std::shared_lock<std::shared_mutex> lock(mapmutex);
    std::optional<BlockType> block = std::nullopt;
    if (map.contains(spot)) {
        block = map.at(spot);
    }


#ifdef MEASURE_LOOKUP
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    cumulative_lookup_time += elapsed;
    lookup_count++;

    if (lookup_count % 1000 == 0) {
        double average_lookup_time = cumulative_lookup_time.count() / lookup_count;
        std::cout << "Average lookup time after " << lookup_count << " lookups: "
                  << average_lookup_time * 1e6 << " microseconds\n";
    }
#endif

    return block;
}

std::optional<BlockType> HashMapDataMap::getLocked(const IntTup& spot) const
{
    std::optional<BlockType> block = std::nullopt;
    if (map.contains(spot)) {
        block = map.at(spot);
    }
    return block;
}

void HashMapDataMap::clear()
{

    map.clear();
}

void HashMapDataMap::set(const IntTup& spot, BlockType block)
{
    //std::cout << "trying to lock mapmutex \n";
    std::unique_lock<std::shared_mutex> lock(mapmutex);
    //std::cout << "locked mapmutex \n";
    map.insert_or_assign(spot, block);
}

void HashMapDataMap::setLocked(const IntTup& spot, BlockType block)
{
    map.insert_or_assign(spot, block);
}
