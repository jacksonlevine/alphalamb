//
// Created by jack on 1/27/2025.
//

#ifndef WORLD_H
#define WORLD_H
#include "DataMap.h"
#include "WorldGenMethod.h"


class World {
public:
    World(DataMap* udm, WorldGenMethod* wm, DataMap* nudm) :
    userDataMap(udm), worldGenMethod(wm), nonUserDataMap(nudm) {};

    DataMap* userDataMap;
    DataMap* nonUserDataMap;

    WorldGenMethod* worldGenMethod;

    uint32_t get(IntTup spot);
    uint32_t getLocked(IntTup spot);




    bool save(std::string filename)
    {
        std::filesystem::path filePath(filename);
        if (!filePath.parent_path().empty()) {
            std::filesystem::create_directories(filePath.parent_path());
        }
        std::ofstream file(filename, std::ios::trunc);

        if (file.is_open()) {

            while (true) {
                if (auto lock = tryToGetReadLockOnDMs()) {
                    const std::unique_ptr<DataMap::Iterator> it = userDataMap->createIterator();
                    while (it->hasNext()) {
                        auto [key, value] = it->next();
                        file << key.x << " " << key.y << " " << key.z << " " << value << '\n';
                    }
                    file.close();
                    return true;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }


        } else {
            std::cout << "Could not open file " << filename << " for writing." << std::endl;
            return false;
        }
    }







    void clearWorld()
    {
        userDataMap->clear();
        nonUserDataMap->clear();
    }
    void setSeed(int seed)
    {
        worldGenMethod->setSeed(seed);
    }
    void set(IntTup spot, uint32_t val);
    inline std::optional<std::pair<std::shared_lock<std::shared_mutex>,
                                 std::shared_lock<std::shared_mutex>>> tryToGetReadLockOnDMs()
    {
        std::shared_lock<std::shared_mutex> lock1(userDataMap->mutex(), std::try_to_lock);
        if (!lock1.owns_lock()) {
            return std::nullopt;
        }

        std::shared_lock<std::shared_mutex> lock2(nonUserDataMap->mutex(), std::try_to_lock);
        if (!lock2.owns_lock()) {
            return std::nullopt;
        }

        return std::make_pair(std::move(lock1), std::move(lock2));
    }
};



#endif //WORLD_H
