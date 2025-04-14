//
// Created by jack on 1/27/2025.
//

#include "World.h"

#include "WorldRenderer.h"
#include "../BlockType.h"


std::optional<std::shared_lock<std::shared_mutex>> tryToGetReadLockOnDM(DataMap* map)
{
    std::shared_lock<std::shared_mutex> lock1(map->mutex(), std::try_to_lock);
    if (!lock1.owns_lock()) {
        return std::nullopt;
    }


    return std::move(lock1);
}

bool loadDM(std::string filename, World* outWorld, entt::registry& reg, BlockAreaRegistry& blockAreas,
            PlacedVoxModelRegistry& pvmr, InvMapKeyedByUID* im,
            std::unordered_set<ClientUID, boost::hash<boost::uuids::uuid>>* existingInvs, const char* regfilename)
{

    loadRegistry(reg, regfilename);

    const bool isClient = im == nullptr;
    if (existingInvs != nullptr)
    {
        existingInvs->clear();
    }
    std::ifstream file(filename);
    if (!file.is_open())
    {

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

                outWorld->set(IntTup(std::stoi(words[0]), std::stoi(words[1]), std::stoi(words[2])),
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

        //Balloon the areas and voxelmodels into the full-on blocks in nonUserDataMap (This is necessary on the server too)
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
            auto lock = outWorld->nonUserDataMap->getUniqueLock();
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
                                outWorld->setNUDMLocked(IntTup{x, y, z}, m.block);
                            }

                        }
                    }
                }
            }

        }


        std::vector<PlacedVoxModel> vms;

        {
            std::shared_lock<std::shared_mutex> lock(outWorld->placedVoxModels.mutex);
            vms.reserve(outWorld->placedVoxModels.models.size());
            for (auto & vm : outWorld->placedVoxModels.models)
            {
                vms.push_back(vm);
            }
        }

        std::vector<IntTup> spotsToEraseInUDM;
        spotsToEraseInUDM.reserve(500);

        {
            auto lock = outWorld->nonUserDataMap->getUniqueLock();

            std::shared_lock<std::shared_mutex> udmRL(outWorld->userDataMap->mutex());
            for (auto & pvm : vms)
            {
                auto & realvm = voxelModels[pvm.name];
                for (auto & p : realvm.points)
                {
                    IntTup offset = IntTup(realvm.dimensions.x/-2, 0, realvm.dimensions.z/-2) + pvm.spot;
                    outWorld->setNUDMLocked(p.localSpot + offset, p.colorIndex);
                    auto bh = outWorld->userDataMap->getUnsafe(p.localSpot + offset);
                    if (bh != std::nullopt && bh.value() == 0)
                    {
                        spotsToEraseInUDM.push_back(p.localSpot + offset);
                    }
                }


            }

        }

        {
            auto lock = outWorld->userDataMap->getUniqueLock();
            for (auto & spot : spotsToEraseInUDM)
            {
                outWorld->userDataMap->erase(spot);
            }
        }
















        return true;
    }
    return false;
}

BlockType World::get(const IntTup& spot)
{
    if (spot.y == 0) { return BEDROCK; }
    return (getRaw(spot) & BLOCK_ID_BITS);
}

BlockType World::getLocked(IntTup spot)
{
    if (spot.y == 0) { return BEDROCK; }
    return (getRawLocked(spot) & BLOCK_ID_BITS);
}

BlockType World::getRaw(IntTup spot)
{
    if (spot.y == 0) { return BEDROCK; }
    // if (auto mem = blockMemo->get(spot); mem != std::nullopt)
    // {
    //     return mem.value();
    // }

    auto id = userDataMap->get(spot);
    if (id == std::nullopt)
    {
        auto nid = nonUserDataMap->get(spot);
        if (nid == std::nullopt)
        {
            return worldGenMethod->get(spot);
        } else
        {
            return nid.value();
        }
    }

    return id.value();
}

BlockType World::getRawLocked(IntTup spot)
{
    if (spot.y == 0) { return BEDROCK; }
    // if (auto mem = blockMemo->get(spot); mem != std::nullopt)
    // {
    //     return mem.value();
    // }

    auto id = userDataMap->getUnsafe(spot);
    if (id == std::nullopt)
    {
        auto nid = nonUserDataMap->getUnsafe(spot);
        if (nid == std::nullopt)
        {
            return worldGenMethod->get(spot);
        } else
        {
            return nid.value();
        }
    }

    return id.value();
}

void World::set(IntTup spot, const BlockType val)
{

    userDataMap->set(spot, val);
    //blockMemo->set(spot, val);
}

void World::setNUDM(const IntTup& spot, const BlockType val)
{
    nonUserDataMap->set(spot, val);
    //blockMemo->set(spot, val);
}
void World::setNUDMLocked(const IntTup& spot, const BlockType val)
{
    nonUserDataMap->setUnsafe(spot, val);
    //blockMemo->set(spot, val);
}

std::optional<std::pair<std::shared_lock<std::shared_mutex>, std::pair<std::shared_lock<std::shared_mutex>, std::
shared_lock<std::shared_mutex>>>> World::tryToGetReadLockOnDMs()
{

    std::shared_lock<std::shared_mutex> lock1(userDataMap->mutex(), std::try_to_lock);
    if (!lock1.owns_lock()) {
        return std::nullopt;
    }

    std::shared_lock<std::shared_mutex> lock2(nonUserDataMap->mutex(), std::try_to_lock);
    if (!lock2.owns_lock()) {
        return std::nullopt;
    }

    std::shared_lock<std::shared_mutex> lock3(lightmapMutex, std::try_to_lock);
    if (!lock3.owns_lock()) {
        return std::nullopt;
    }

    return std::make_pair(std::move(lock1), std::make_pair(std::move(lock2), std::move(lock3)));

}

std::optional<std::pair<std::shared_lock<std::shared_mutex>, std::pair<std::shared_lock<std::shared_mutex>, std::
unique_lock<std::shared_mutex>>>> World::tryToGetReadLockOnDMsAndWriteLockOnLM()
{

    std::shared_lock<std::shared_mutex> lock1(userDataMap->mutex(), std::try_to_lock);
    if (!lock1.owns_lock()) {
        return std::nullopt;
    }

    std::shared_lock<std::shared_mutex> lock2(nonUserDataMap->mutex(), std::try_to_lock);
    if (!lock2.owns_lock()) {
        return std::nullopt;
    }

    std::unique_lock<std::shared_mutex> lock3(lightmapMutex, std::try_to_lock);
    if (!lock3.owns_lock()) {
        return std::nullopt;
    }

    return std::make_pair(std::move(lock1), std::make_pair(std::move(lock2), std::move(lock3)));

}
