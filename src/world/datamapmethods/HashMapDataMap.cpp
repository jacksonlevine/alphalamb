#include "HashMapDataMap.h"
#include "../../BlockType.h"
#include <chrono>

HashMapDataMapIterator::HashMapDataMapIterator(HashMapDataMap& dataMap)
    : dataMap(dataMap),
    chunkIt(dataMap.chunks.begin()),
    chunkEnd(dataMap.chunks.end())
{
    if (chunkIt != chunkEnd) {
        blockIt = chunkIt->second.begin();
        blockEnd = chunkIt->second.end();
        advanceToNext();
    }
}

bool HashMapDataMapIterator::hasNext() const {
    return chunkIt != chunkEnd;
}

std::pair<const IntTup&, BlockType&> HashMapDataMapIterator::next() {
    if (!hasNext()) throw std::out_of_range("Iterator out of range");

    // The full position is already stored, no need to transform
    currentPair = *blockIt;

    // Move to next element
    ++blockIt;
    advanceToNext();

    return currentPair;
}

void HashMapDataMapIterator::advanceToNext() {
    // If we've reached the end of blocks in the current chunk
    while (chunkIt != chunkEnd && blockIt == blockEnd) {
        ++chunkIt;
        if (chunkIt != chunkEnd) {
            blockIt = chunkIt->second.begin();
            blockEnd = chunkIt->second.end();
        }
    }
}

void HashMapDataMap::erase(const IntTup& spot, bool locked)
{
    auto chunkPos = getChunkPos(spot);

    if (!locked)
    {
        std::unique_lock<std::shared_mutex> lock(mapmutex);
        if (chunks.find(chunkPos) != chunks.end()) {
            chunks[chunkPos].erase(spot);
            // Remove empty chunks
            if (chunks[chunkPos].empty()) {
                chunks.erase(chunkPos);
            }
        }
    }
    else
    {
        if (chunks.find(chunkPos) != chunks.end()) {
            chunks[chunkPos].erase(spot);
            // Remove empty chunks
            if (chunks[chunkPos].empty()) {
                chunks.erase(chunkPos);
            }
        }
    }
}

void HashMapDataMap::erase(const TwoIntTup& spot, bool locked)
{
    auto chunkPos = spot;

    if (!locked)
    {
        std::unique_lock<std::shared_mutex> lock(mapmutex);
        if (chunks.find(chunkPos) != chunks.end()) {
            chunks.erase(chunkPos);
        }
    }
    else
    {
        if (chunks.find(chunkPos) != chunks.end()) {
            chunks.erase(chunkPos);
        }
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

    auto chunkPos = getChunkPos(spot);

    std::optional<BlockType> block = std::nullopt;
    auto chunkIt = chunks.find(chunkPos);
    if (chunkIt != chunks.end()) {
        auto blockIt = chunkIt->second.find(spot);
        if (blockIt != chunkIt->second.end()) {
            block = blockIt->second;
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
    return block;
}

std::optional<BlockType> HashMapDataMap::getUnsafe(const IntTup& spot) const
{
    auto chunkPos = getChunkPos(spot);

    std::optional<BlockType> block = std::nullopt;
    auto chunkIt = chunks.find(chunkPos);
    if (chunkIt != chunks.end()) {
        auto blockIt = chunkIt->second.find(spot);
        if (blockIt != chunkIt->second.end()) {
            block = blockIt->second;
        }
    }

    return block;
}

void HashMapDataMap::clear()
{
    chunks.clear();
}

void HashMapDataMap::set(const IntTup& spot, BlockType block)
{
    std::unique_lock<std::shared_mutex> lock(mapmutex);

    auto chunkPos = getChunkPos(spot);
    chunks[chunkPos][spot] = block;
}

void HashMapDataMap::setUnsafe(const IntTup& spot, BlockType block)
{
    auto chunkPos = getChunkPos(spot);
    chunks[chunkPos][spot] = block;
}