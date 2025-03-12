//
// Created by jack on 1/27/2025.
//

#ifndef WORLD_H
#define WORLD_H
#include "DataMap.h"
#include "VoxelModels.h"
#include "WorldGenMethod.h"
#include "../PlayerInfoMapKeyedByUID.h"
#include "datamapmethods/HashMapDataMap.h"


constexpr BlockType BLOCK_ID_BITS = 0b0000'0000'0000'0000'1111'1111'1111'1111;
constexpr BlockType BLOCK_FLAG_BITS = 0b1111'1111'1111'1111'0000'0000'0000'0000;

constexpr BlockType BLOCK_DIRECTION_BITS = 0b0000'0000'0000'0011'0000'0000'0000'0000;

constexpr BlockType CONNECT_X_BIT =    0b1000'0000'0000'0000'0000'0000'0000'0000;
constexpr BlockType CONNECT_NEGX_BIT = 0b0100'0000'0000'0000'0000'0000'0000'0000;
constexpr BlockType CONNECT_Z_BIT =    0b0010'0000'0000'0000'0000'0000'0000'0000;
constexpr BlockType CONNECT_NEGZ_BIT = 0b0001'0000'0000'0000'0000'0000'0000'0000;



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
                if((value & BLOCK_ID_BITS) == FENCE)
                {
                    std::cout << "A fence with id: " <<  << " 0x" << std::hex << (uint32_t)value << std::dec << '\n';
                }
                contentStream << key.x << " " << key.y << " " << key.z << " 0x" << std::hex << (uint32_t)value << std::dec << '\n';
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
            contentStream << "INVOMAX " << to_string(id) << " ";
            for (auto & item : inventory.inventory)
            {
                contentStream << std::to_string(item.block) << " " << std::to_string((int)item.count) << " " << std::to_string(item.isItem) << " ";
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


   inline bool loadDM(std::string filename, DataMap* map, BlockAreaRegistry& blockAreas, PlacedVoxModelRegistry& pvmr, InvMapKeyedByUID* im = nullptr, std::unordered_set<ClientUID, boost::hash<boost::uuids::uuid>>* existingInvs = nullptr)
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
                                auto uid = gen(word);
                                existingInvs->insert(uid);
                            }
                            break;
                        }
                    }
                }
                if (words.size() == 4)
                {

                    map->set(IntTup(std::stoi(words[0]), std::stoi(words[1]), std::stoi(words[2])),
                    static_cast<BlockType>(std::stoul(words[3], nullptr, 0)));
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
                            slot.block = static_cast<BlockType>(std::stoul(words[index + 0]));
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

inline std::optional<Inventory> loadInvFromFile(std::string filename, ClientUID id)
{
    auto result = std::nullopt;
    std::ifstream file(filename);
    if (!file.is_open())
        return result;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.starts_with("INVOMAX"))
        {
            std::istringstream iss(line);
            std::string word;
            iss >> word;
            iss >> word;
            boost::uuids::string_generator gen{};
            if (gen(word) == id)
            {
                std::cout << "Found a matching inv! \n";

                //Get all the rest of the words since its mtaching
                std::vector<std::string> words;
                while (iss >> word)
                {
                   // std::cout << "Word: " << word << "\n";
                    words.push_back(word);
                }

                Inventory inv = {};
                int indexo = 0;
                for (auto & slot : inv.inventory)
                {
                    int index = (indexo * 3);
                    slot.block = static_cast<BlockType>(std::stoul(words[index + 0]));
                    slot.count = std::stoi(words[index + 1]);
                    slot.isItem = std::stoi(words[index + 2]);
                    indexo += 1;
                }
                return inv;
            }

        }
    }

    return result;
}




class World {
public:
    World(DataMap* udm, WorldGenMethod* wm, DataMap* nudm) :
    userDataMap(udm), worldGenMethod(wm), nonUserDataMap(nudm) {};

    DataMap* userDataMap;
    DataMap* nonUserDataMap;

    BlockAreaRegistry blockAreas = {};
    PlacedVoxModelRegistry placedVoxModels = {};

    DataMap* blockMemo = new HashMapDataMap();

    WorldGenMethod* worldGenMethod;

    BlockType get(const IntTup& spot);
    BlockType getLocked(IntTup spot);

    BlockType getRaw(IntTup spot);
    BlockType getRawLocked(IntTup spot);


    bool load(std::string filename, std::unordered_set<ClientUID, boost::hash<boost::uuids::uuid>>& existingInvs)
    {
        if (loadDM(filename, userDataMap, blockAreas, placedVoxModels, nullptr, &existingInvs))
        {

            //Balloon the areas and voxelmodels into the full-on blocks in nonUserDataMap (This is not at all necessary on the server)
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
                                    setNUDMLocked(IntTup{x, y, z}, m.block);
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
                        setNUDMLocked(p.localSpot + offset, p.colorIndex);
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
    void setNUDM(const IntTup& spot, BlockType val);
    void setNUDMLocked(const IntTup& spot, BlockType val);
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
