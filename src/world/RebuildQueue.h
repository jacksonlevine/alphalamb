//
// Created by jack on 2/6/2025.
//

#ifndef REBUILDQUEUE_H
#define REBUILDQUEUE_H

#include <queue>

#include "../PrecompHeader.h"

// Add these to WorldRenderer.h class definition
struct ChunkRebuildRequest {
    TwoIntTup chunkPos;
    size_t chunkIndex = 0 ;
    bool isHighPriority;
    bool rebuild = true;
    IntTup changeSpot = IntTup(0,0,0);

    bool isArea = false;
    BlockArea area;


    std::optional<uint32_t> changeTo = std::nullopt;
    std::chrono::steady_clock::time_point timestamp;
    ChunkRebuildRequest(TwoIntTup pos, size_t index, bool priority)
        : chunkPos(pos), chunkIndex(index), isHighPriority(priority),
          timestamp(std::chrono::steady_clock::now()) {}
    ChunkRebuildRequest(BlockArea area)
        : chunkPos(TwoIntTup(0,0)), chunkIndex(0), isHighPriority(true), isArea(true), area(area),
          timestamp(std::chrono::steady_clock::now()) {}
    ChunkRebuildRequest(TwoIntTup pos, size_t index, bool priority, IntTup changeSpot, uint32_t changeTo)
        : chunkPos(pos), chunkIndex(index), isHighPriority(priority), changeSpot(changeSpot), changeTo(changeTo),
          timestamp(std::chrono::steady_clock::now()) {}
    ChunkRebuildRequest(TwoIntTup pos, size_t index, bool priority, IntTup changeSpot, uint32_t changeTo, bool rebuild)
        : chunkPos(pos), chunkIndex(index), isHighPriority(priority), changeSpot(changeSpot), changeTo(changeTo), rebuild(rebuild),
          timestamp(std::chrono::steady_clock::now()) {}
    ChunkRebuildRequest()
        : chunkPos(0, 0), chunkIndex(0), isHighPriority(false), timestamp(std::chrono::steady_clock::now()) {}

};

// Priority queue comparator
struct RebuildRequestCompare {
    bool operator()(const ChunkRebuildRequest& a, const ChunkRebuildRequest& b) {
        if (a.isHighPriority != b.isHighPriority) {
            return !a.isHighPriority; // High priority comes first
        }
        return a.timestamp > b.timestamp; // Older requests come first within same priority
    }
};

// Thread-safe priority queue for rebuild requests
class RebuildQueue {
private:
    std::priority_queue<ChunkRebuildRequest,
                        std::vector<ChunkRebuildRequest>,
                        RebuildRequestCompare> queue;
    std::mutex mutex;
    std::condition_variable cv;


public:
    bool shouldExit = false;
    void push(const ChunkRebuildRequest& request) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(request);
        cv.notify_one();
    }

    bool pop(ChunkRebuildRequest& request) {
        //std::cout << "Trying to lock queue \n";
        std::unique_lock<std::mutex> lock(mutex);
        //std::cout << "Locked queue \n";
        while (queue.empty() && !shouldExit) {
            cv.wait_for(lock, std::chrono::milliseconds(100));
        }

        if (queue.empty()) return false;

        request = queue.top();
        queue.pop();
        return true;
    }

    void signalExit() {
        std::lock_guard<std::mutex> lock(mutex);
        shouldExit = true;
        cv.notify_all();
    }
};


#endif //REBUILDQUEUE_H


