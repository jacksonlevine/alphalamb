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
    size_t chunkIndex = 0;
    bool isHighPriority;
    bool rebuild = true;
    IntTup changeSpot = IntTup(0,0,0);

    bool isArea = false;
    BlockArea area;

    bool isVoxelModel = false;
    PlacedVoxModel vm = {};
    bool doLightPass = false;
    bool queueLightpassImplicated = true;

    glm::vec3 playerPosAtPlaceTime = glm::vec3(0.f);

    std::optional<BlockType> changeTo = std::nullopt;
    std::chrono::steady_clock::time_point timestamp;
    ChunkRebuildRequest(TwoIntTup pos, size_t index, bool priority)
        : chunkPos(pos), chunkIndex(index), isHighPriority(priority),
          timestamp(std::chrono::steady_clock::now()) {}
    ChunkRebuildRequest(BlockArea area)
        : chunkPos(TwoIntTup(0,0)), isHighPriority(true), isArea(true), area(area),
          timestamp(std::chrono::steady_clock::now()) {}
    ChunkRebuildRequest(PlacedVoxModel vm)
        : chunkPos(TwoIntTup(0,0)), isHighPriority(true), isVoxelModel(true), vm(vm),
          timestamp(std::chrono::steady_clock::now()) {}
    ChunkRebuildRequest(TwoIntTup pos, size_t index, bool priority, IntTup changeSpot, BlockType changeTo, const glm::vec3& playerPosAtPlaceTime)
        : chunkPos(pos), chunkIndex(index), isHighPriority(priority), changeSpot(changeSpot), changeTo(changeTo), playerPosAtPlaceTime(playerPosAtPlaceTime),
          timestamp(std::chrono::steady_clock::now()) {}
    ChunkRebuildRequest(TwoIntTup pos, size_t index, bool priority, IntTup changeSpot, BlockType changeTo, bool rebuild, const glm::vec3& playerPosAtPlaceTime)
        : chunkPos(pos), chunkIndex(index), isHighPriority(priority), changeSpot(changeSpot), changeTo(changeTo), rebuild(rebuild), playerPosAtPlaceTime(playerPosAtPlaceTime),
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

    std::mutex cvmutex;
    std::condition_variable cv;

    std::shared_mutex queueMutex;
    std::mutex overallMutex;

public:
    bool shouldExit = false;
    void push(const ChunkRebuildRequest& request) {
        {
            auto ol = std::unique_lock<std::mutex>(overallMutex);
            {
                auto lock = std::unique_lock<std::shared_mutex>(queueMutex);
                queue.push(request);
            }
            cv.notify_one();
        }
    }

    bool pop(ChunkRebuildRequest& request) {
        //std::cout << "Trying to lock queue \n";

        //std::cout << "Locked queue \n";

        std::unique_lock<std::mutex> cvlock(cvmutex);

        bool nocontinue = false;
        bool queueempty = false;
        {
            std::shared_lock<std::shared_mutex> qlock(queueMutex);
            queueempty = queue.empty();
            nocontinue = queueempty && !shouldExit;
        }

        while (nocontinue) {
            {
                std::shared_lock<std::shared_mutex> qlock(queueMutex);
                queueempty = queue.empty();
                nocontinue = queueempty && !shouldExit;
            }
            cv.wait_for(cvlock, std::chrono::milliseconds(100));
        }

        if (queueempty) return false;

        {
            auto ol = std::unique_lock<std::mutex>(overallMutex);
            {
                std::shared_lock<std::shared_mutex> qlock(queueMutex);
                request = queue.top();
                queue.pop();
            }
        }
        return true;
    }

    void signalExit() {
        std::lock_guard<std::mutex> lock(cvmutex);
        shouldExit = true;
        cv.notify_all();
    }
};


#endif //REBUILDQUEUE_H


