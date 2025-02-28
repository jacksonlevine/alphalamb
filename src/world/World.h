//
// Created by jack on 1/27/2025.
//

#ifndef WORLD_H
#define WORLD_H
#include "DataMap.h"
#include "VoxelModels.h"
#include "WorldGenMethod.h"
#include "../PlayerInfoMapKeyedByUID.h"


constexpr BlockType BLOCK_ID_BITS = 0b0000'0000'0000'0000'1111'1111'1111'1111;
constexpr BlockType BLOCK_FLAG_BITS = 0b1111'1111'1111'1111'0000'0000'0000'0000;

constexpr BlockType BLOCK_DIRECTION_BITS = 0b0000'0000'0000'0011'0000'0000'0000'0000;

constexpr BlockType getDirectionBits(const BlockType& input) {
    return (input & BLOCK_DIRECTION_BITS) >> 16;
};

constexpr void setDirectionBits(BlockType& inout, const BlockType& direction) {

    BlockType bits = direction << 16;
    inout |= bits;

};


inline std::optional<std::shared_lock<std::shared_mutex>> tryToGetReadLockOnDM(DataMap* map)
{
    std::shared_lock<std::shared_mutex> lock1(map->mutex(), std::try_to_lock);
    if (!lock1.owns_lock()) {
        return std::nullopt;
    }


    return std::move(lock1);
}

struct BlockArea
{
    IntTup corner1;
    IntTup corner2;
    BlockType block;
    bool hollow = false;
};

struct BlockAreaRegistry
{
    std::vector<BlockArea> blockAreas = {};
    std::shared_mutex baMutex = {};
};

struct PlacedVoxModel
{
    VoxelModelName name;
    IntTup spot;
};

struct PlacedVoxModelRegistry
{
    std::vector<PlacedVoxModel> models;
    std::shared_mutex mutex = {};
};


inline std::optional<std::string> saveDM(std::string filename, DataMap* map, BlockAreaRegistry& blockAreas, PlacedVoxModelRegistry& pvmr, InvMapKeyedByUID& im) {
    std::filesystem::path filePath(filename);
    if (!filePath.parent_path().empty()) {
        std::filesystem::create_directories(filePath.parent_path());
    }

    std::ostringstream contentStream; // String stream to store content before writing to file
    bool gotlock = false;
    while (!gotlock) {
        if (auto lock = tryToGetReadLockOnDM(map)) {
            const std::unique_ptr<DataMap::Iterator> it = map->createIterator();
            while (it->hasNext()) {
                auto [key, value] = it->next();
                contentStream << key.x << " " << key.y << " " << key.z << " " << (int)value << '\n';
            }
            gotlock = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    {
        std::shared_lock<std::shared_mutex> lock(blockAreas.baMutex);
        if (!blockAreas.blockAreas.empty())
        {
            for (auto& area : blockAreas.blockAreas)
            {
                contentStream << "AREA " << area.corner1.x << " " << area.corner1.y << " " << area.corner1.z << " "
                << area.corner2.x << " " << area.corner2.y << " " << area.corner2.z << " "
                << (int)area.block << " " << (int)area.hollow << '\n';
            }
        }
    }

    {
        std::shared_lock<std::shared_mutex> lock(pvmr.mutex);
        if (!pvmr.models.empty())
        {
            for (auto& vm : pvmr.models)
            {
                contentStream << "VM " << vm.name << " " << vm.spot.x << " " << vm.spot.y << " " << vm.spot.z << "\n";
            }
        }
    }

    {
        auto read = std::shared_lock<std::shared_mutex>(im.rw);
        for (auto &[id, inventory] : im.invMap)
        {
            contentStream << "INVOMAX " << id << " ";
            for (auto & item : inventory.inventory)
            {
                contentStream << item.block << " " << item.count << " " << item.isItem << " ";
            }
            contentStream << '\n';
        }
    }

    std::ofstream file(filename, std::ios::trunc);
    if (file.is_open()) {
        file << contentStream.str();
        file.close();
        return contentStream.str(); // Return the written content
    } else {
        std::cerr << "Could not open file " << filename << " for writing." << std::endl;
        return std::nullopt;
    }
}



   inline bool loadDM(std::string filename, DataMap* map, BlockAreaRegistry& blockAreas, PlacedVoxModelRegistry& pvmr, InvMapKeyedByUID* im = nullptr, std::unordered_set<ClientUID>* existingInvs = nullptr)
    {
        const bool isClient = im == nullptr;
        if (existingInvs != nullptr)
        {
            existingInvs->clear();
        }
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cout << "Could not open file " << filename << " for reading." << std::endl;
            return false;
        } else
        {
            std::string line;
            while (std::getline(file, line))
            {
                std::istringstream iss(line);
                std::vector<std::string> words;
                std::string word;
                {
                    while (iss >> word)
                    {
                        words.push_back(word);
                        if (isClient && word == "INVOMAX")
                        {
                            iss >> word;
                            boost::uuids::string_generator gen{};
                            if (existingInvs != nullptr)
                            {
                                existingInvs->insert(gen(word));
                            }
                            break;
                        }
                    }
                }
                if (words.size() == 4)
                {
                    //Read the block (words[3]) as unsigned long because that can contain all the uint32_t values
                    //Rest are ints
                    map->set(IntTup( std::stoi(words[0]) , std::stoi(words[1]), std::stoi(words[2]) ), static_cast<BlockType>(std::stoul(words[3])));
                } else if (words.size() == 5 && words.at(0) == "VM")
                {
                    std::unique_lock<std::shared_mutex> lock(pvmr.mutex);
                    pvmr.models.push_back(PlacedVoxModel{
                    (VoxelModelName)std::stoi(words[1]), IntTup(std::stoi(words[2]), std::stoi(words[3]), std::stoi(words[4])),});

                } else if (words.size() && words.at(0) == "AREA")
                {
                    if (words.size() == 8 && words.at(0) == "AREA")
                    {
                        std::unique_lock<std::shared_mutex> lock(blockAreas.baMutex);
                        blockAreas.blockAreas.push_back(BlockArea{
                            .corner1 = IntTup(std::stoi(words[1]),std::stoi(words[2]),std::stoi(words[3])),
                            .corner2 = IntTup(std::stoi(words[4]),std::stoi(words[5]),std::stoi(words[6])),
                            .block = static_cast<BlockType>(std::stoul(words[7])),
                            .hollow = false
                        });
                    }
                    if (words.size() == 9 && words.at(0) == "AREA")
                    {
                        std::unique_lock<std::shared_mutex> lock(blockAreas.baMutex);
                        blockAreas.blockAreas.push_back(BlockArea{
                            .corner1 = IntTup(std::stoi(words[1]),std::stoi(words[2]),std::stoi(words[3])),
                            .corner2 = IntTup(std::stoi(words[4]),std::stoi(words[5]),std::stoi(words[6])),
                            .block = static_cast<BlockType>(std::stoul(words[7])),
                            .hollow = (bool)std::stoi(words[8]),
                        });
                    }
                } else if (words.size() && words.at(0) == "INVOMAX")
                {
                    if (!isClient)
                    {
                        boost::uuids::string_generator gen{};
                        auto inv = im->getWrite(gen(words.at(1)));
                        int indexo = 0;
                        for (auto & slot : inv.second.inventory)
                        {
                            int index = (indexo * 3) + 2;
                            slot.block = static_cast<BlockType>(std::stoul(words[index]));
                            slot.count = std::stoi(words[index + 1]);
                            slot.isItem = std::stoi(words[index + 2]);
                            indexo += 1;
                        }
                    }
                }


            }
            file.close();
            return true;
        }
    return false;
    }




class World {
public:
    World(DataMap* udm, WorldGenMethod* wm, DataMap* nudm) :
    userDataMap(udm), worldGenMethod(wm), nonUserDataMap(nudm) {};

    DataMap* userDataMap;
    DataMap* nonUserDataMap;

    BlockAreaRegistry blockAreas = {};
    PlacedVoxModelRegistry placedVoxModels = {};



    WorldGenMethod* worldGenMethod;

    BlockType get(const IntTup& spot);
    BlockType getLocked(IntTup spot);

    BlockType getRaw(IntTup spot);
    BlockType getRawLocked(IntTup spot);


    bool load(std::string filename)
    {
        if (loadDM(filename, userDataMap, blockAreas, placedVoxModels))
        {
            std::vector<BlockArea> ba;

            {
                std::shared_lock<std::shared_mutex> lock(blockAreas.baMutex);
                ba.reserve(blockAreas.blockAreas.size());
                for (auto & area : blockAreas.blockAreas)
                {
                    ba.push_back(area);
                }
            }

            {
                auto lock = nonUserDataMap->getUniqueLock();
                for (auto & m : ba)
                {
                    int minX = std::min(m.corner1.x, m.corner2.x);
                    int maxX = std::max(m.corner1.x, m.corner2.x);
                    int minY = std::min(m.corner1.y, m.corner2.y);
                    int maxY = std::max(m.corner1.y, m.corner2.y);
                    int minZ = std::min(m.corner1.z, m.corner2.z);
                    int maxZ = std::max(m.corner1.z, m.corner2.z);


                    for (int x = minX; x <= maxX; x++) {
                        for (int y = minY; y <= maxY; y++) {
                            for (int z = minZ; z <= maxZ; z++) {

                                bool isBoundary = (x == minX || x == maxX ||
                                                       y == minY || y == maxY ||
                                                       z == minZ || z == maxZ);
                                if (isBoundary || !m.hollow)
                                {
                                    nonUserDataMap->setLocked(IntTup{x, y, z}, m.block);
                                }

                            }
                        }
                    }
                }

            }


            std::vector<PlacedVoxModel> vms;

            {
                std::shared_lock<std::shared_mutex> lock(placedVoxModels.mutex);
                vms.reserve(placedVoxModels.models.size());
                for (auto & vm : placedVoxModels.models)
                {
                    vms.push_back(vm);
                }
            }

            std::vector<IntTup> spotsToEraseInUDM;
            spotsToEraseInUDM.reserve(500);

            {
                auto lock = nonUserDataMap->getUniqueLock();

                std::shared_lock<std::shared_mutex> udmRL(userDataMap->mutex());
                for (auto & pvm : vms)
                {
                    auto & realvm = voxelModels[pvm.name];
                    for (auto & p : realvm.points)
                    {
                        IntTup offset = IntTup(realvm.dimensions.x/-2, 0, realvm.dimensions.z/-2) + pvm.spot;
                        nonUserDataMap->setLocked(p.localSpot + offset, p.colorIndex);
                        auto bh = userDataMap->getLocked(p.localSpot + offset);
                        if (bh != std::nullopt && bh.value() == 0)
                        {
                            spotsToEraseInUDM.push_back(p.localSpot + offset);
                        }
                    }


                }

            }

            {
                auto lock = userDataMap->getUniqueLock();
                for (auto & spot : spotsToEraseInUDM)
                {
                    userDataMap->erase(spot);
                }
            }




            return true;
        };
        return false;
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
    void set(IntTup spot, BlockType val);

    std::optional<std::pair<std::shared_lock<std::shared_mutex>,
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
