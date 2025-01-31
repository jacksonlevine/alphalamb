#include "HashMapDataMap.h"

std::optional<uint32_t> HashMapDataMap::get(IntTup spot) const {

#ifdef MEASURE_LOOKUP
    auto start = std::chrono::high_resolution_clock::now();
#endif

    std::shared_lock<std::shared_mutex> lock(mutex);
    std::optional<uint32_t> block = std::nullopt;
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

void HashMapDataMap::set(IntTup spot, uint32_t block)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    map.insert_or_assign(spot, block);
}
