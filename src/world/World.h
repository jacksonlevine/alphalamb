//
// Created by jack on 1/27/2025.
//

#ifndef WORLD_H
#define WORLD_H
#include "DataMap.h"
#include "VoxelModels.h"
#include "WorldGenMethod.h"
#include "../PlayerInfoMapKeyedByUID.h"
#include "../SaveRegistry.h"
#include "datamapmethods/HashMapDataMap.h"


class World;
constexpr BlockType BLOCK_ID_BITS = 0b0000'0000'0000'0000'1111'1111'1111'1111;
constexpr BlockType BLOCK_FLAG_BITS = 0b1111'1111'1111'1111'0000'0000'0000'0000;

constexpr BlockType BLOCK_DIRECTION_BITS = 0b0000'0000'0000'0011'0000'0000'0000'0000;

constexpr BlockType CONNECT_X_BIT =    0b1000'0000'0000'0000'0000'0000'0000'0000;
constexpr BlockType CONNECT_NEGX_BIT = 0b0100'0000'0000'0000'0000'0000'0000'0000;
constexpr BlockType CONNECT_Z_BIT =    0b0010'0000'0000'0000'0000'0000'0000'0000;
constexpr BlockType CONNECT_NEGZ_BIT = 0b0001'0000'0000'0000'0000'0000'0000'0000;
constexpr BlockType CONNECT_Y_BIT =    0b0000'1000'0000'0000'0000'0000'0000'0000;
constexpr BlockType CONNECT_NEGY_BIT = 0b0000'0100'0000'0000'0000'0000'0000'0000;


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


inline std::optional<std::string> saveDM(std::string filename, DataMap* map, BlockAreaRegistry& blockAreas, PlacedVoxModelRegistry& pvmr, InvMapKeyedByUID& im, entt
                                         ::registry& reg, const char* regsnapshotfilename) {
    std::filesystem::path filePath(filename);
    if (!filePath.parent_path().empty()) {
        std::filesystem::create_directories(filePath.parent_path());
    }

    // {
    //     using namespace entt;
    //     output_archive output;
    //
    //     entt::snapshot{reg}
    //     .get<entt::entity>(output)
    //     .get<a_component>(output)
    //     .get<another_component>(output);
    // }

    saveRegistry(reg, regsnapshotfilename);


    std::ostringstream contentStream; // String stream to store content before writing to file
    bool gotlock = false;
    while (!gotlock) {
        if (auto lock = tryToGetReadLockOnDM(map)) {
            const std::unique_ptr<DataMap::Iterator> it = map->createIterator();
            while (it->hasNext()) {
                auto [key, value] = it->next();
                // if((value & BLOCK_ID_BITS) == STONE_STAIRS)
                // {
                //     std::cout << "A stone stair with id: " << value << " Hex: "  << " 0x" << std::hex << (uint32_t)value << std::dec << '\n';
                // }
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


bool loadDM(std::string filename, World* outWorld, entt::registry& reg, BlockAreaRegistry& blockAreas, PlacedVoxModelRegistry& pvmr, InvMapKeyedByUID* im = nullptr, std::unordered_set<ClientUID, boost::hash<boost::uuids::uuid>>* existingInvs = nullptr, const
                      char* regfilename = "snapshot.bin");

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


    bool load(std::string filename, std::unordered_set<ClientUID, boost::hash<boost::uuids::uuid>>& existingInvs, entt::registry& reg)
    {
        if (loadDM(filename, this, reg, blockAreas, placedVoxModels, nullptr, &existingInvs))
        {




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
