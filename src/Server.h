//
// Created by jack on 2/9/2025.
//

#ifndef SERVER_H
#define SERVER_H

#include "Camera.h"
#include "LocalServerIOContext.h"
#include "PrecompHeader.h"
#include "NetworkTypes.h"
#include "PlayerInfoMapKeyedByUID.h"
#include "components/Factories.h"
#include "components/PlayerEmplacer.h"
#include "components/UUIDComponent.h"
#include "components/WorldStateComponent.h"
#include "specialblocks/FindEntityCreateFunc.h"
#include "specialblocks/FindSpecialBlock.h"
#include "world/DataMap.h"
#include "world/World.h"
#include "FlatCposSet.h"
#include "Recipes.h"
#include "components/Lifetime.h"
using tcp = boost::asio::ip::tcp;


struct ServerPlayer
{
    std::weak_ptr<tcp::socket> socket;
    Controls controls;
    jl::Camera camera;
    bool receivedWorld = false;
    ClientUID myUID = {};
};

extern std::unordered_map<entt::entity, ServerPlayer> clients;

extern std::shared_mutex clientsMutex;
//
// extern BlockAreaRegistry serverWorld.blockAreas;
// extern PlacedVoxModelRegistry serverWorld.placedVoxModels;
// extern DataMap* serverWorld.userDataMap;

extern World serverWorld;
extern InvMapKeyedByUID invMapKeyedByUID;
extern entt::registry serverReg;



constexpr int DGSEEDSEED = 987654321;

inline void sendMessageToAllClients(const DGMessage& message, entt::entity m_playerIndex, bool excludeMe)
{
    std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);

    auto view = serverReg.view<NetworkComponent>();
    for (auto entity : view)
    {
        auto nc = view.get<NetworkComponent>(entity);
        if(!((entity == m_playerIndex) && excludeMe) && !nc.socket.expired() && nc.receivedWorld)
        {
            boost::asio::write(*nc.socket.lock(), boost::asio::buffer(&message, sizeof(DGMessage)));
        }
    }
}

inline std::vector<char> loadBinaryFile(const std::string& filename) {
    if (std::filesystem::exists(filename))
    {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return {};
        }

        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(fileSize);
        file.read(buffer.data(), fileSize);

        return buffer;
    } else
    {

    }
    return std::vector<char>();
}

class Session : public std::enable_shared_from_this<Session>
{
public:
    std::weak_ptr<flatCposSet> generatedChunks;



    explicit Session(std::shared_ptr<tcp::socket> socket, std::shared_ptr<flatCposSet> generatedChunks)
    : m_socket(std::move(socket)), m_playerIndex(entt::null), generatedChunks(generatedChunks) { }

    void run() {
        sayInitialThings();
    }
private:

    std::pmr::synchronized_pool_resource m_pool;
    std::pmr::polymorphic_allocator<DGMessage> m_alloc;

    void sayInitialThings()
    {
        auto self(shared_from_this());


        //Client needs to send their greeting before they get anything.
        ClientToServerGreeting playerInit = {};
        boost::asio::read(*m_socket, boost::asio::buffer(&playerInit, sizeof(DGMessage)));

        m_clientUID = playerInit.id;

        {
            std::shared_lock<std::shared_mutex> clientslock(clientsMutex);


            auto view = serverReg.view<UUIDComponent>();
            for (auto entity : view)
            {
                auto comp = view.get<UUIDComponent>(entity);
                if (m_clientUID == comp.uuid)
                {

                    m_playerIndex = entity;
                    break;
                }

            }
            if (m_playerIndex == entt::null)
            {

                m_playerIndex = serverReg.create();
                emplacePlayerParts(serverReg, m_playerIndex, m_clientUID);

            }


            // clients.insert({index, ServerPlayer{
            //     .socket = shared_socket,
            //     .controls = Controls{},
            //     .camera = jl::Camera{},
            //     .receivedWorld = std::atomic<bool>{false},
            // }});
        }

        {
            std::unique_lock<std::shared_mutex> clientslock(clientsMutex);
            auto & nw = serverReg.get<NetworkComponent>(m_playerIndex);
            nw.socket = m_socket;
        }





        //
        // {
        //     std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
        //     clients.at(m_playerIndex).myUID = m_clientUID;
        // }

        //Now the players inv will exist in the world they download

        auto string = saveDM("world/serverworld.txt", &serverWorld.userDataMap, serverWorld.blockAreas, serverWorld.placedVoxModels, invMapKeyedByUID, serverReg, "world/serversnap.bin");
        if (string.has_value())
        {

            auto regfile = loadBinaryFile("world/serversnap.bin");

            DGMessage fileInit = FileTransferInit {
            .fileSize = string.value().size() * sizeof(char),
            .isWorld  = true, .regFileSize = regfile.size(),};
            boost::asio::write(*m_socket, boost::asio::buffer(&fileInit, sizeof(DGMessage))
            );

            boost::asio::write(*m_socket, boost::asio::buffer(string.value().data(), string.value().size() * sizeof(char))
            );

            //Write the registry snapshot

            boost::asio::write(*m_socket, boost::asio::buffer(regfile.data(), regfile.size() * sizeof(char)));
        }


        DGMessage wi = WorldInfo {
            .seed = DGSEEDSEED,
            .yourPosition = glm::vec3(0, 200, 0),
            .yourPlayerIndex = m_playerIndex
        };

        boost::asio::write(*m_socket, boost::asio::buffer(&wi, sizeof(DGMessage)));




        {
            std::shared_lock<std::shared_mutex> clientsLock(clientsMutex);


            auto view = serverReg.view<NetworkComponent, UUIDComponent, jl::Camera>();

            for (auto entity : view)
            {
                auto nc = view.get<NetworkComponent>(entity);
                auto id = entity;
                auto camera = view.get<jl::Camera>(entity);
                auto myUID = view.get<UUIDComponent>(entity).uuid;

                if(id != m_playerIndex)
                {
                    DGMessage playerPresent = PlayerPresent {
                        .index = id, .position = camera.transform.position, .direction = camera.transform.direction,
                        .id = myUID};
                    //We are notified of this person
                    boost::asio::write(*m_socket, boost::asio::buffer(&playerPresent, sizeof(DGMessage)));
                    //And also notify this person we are here
                    if(!nc.socket.expired() && nc.receivedWorld)
                    {
                        DGMessage ourPlayerPresent = PlayerPresent {
                            .index = m_playerIndex, .position = serverReg.get<jl::Camera>(m_playerIndex).transform.position,
                            .direction = serverReg.get<jl::Camera>(m_playerIndex).transform.direction, .id = m_clientUID
                        };

                        boost::asio::write(*nc.socket.lock(), boost::asio::buffer(&ourPlayerPresent, sizeof(DGMessage)));
                    }
                }
            }
            serverReg.get<NetworkComponent>(m_playerIndex).receivedWorld = true;
        }



        //Now start waiting for messages
        waitForMessage();
    }

    void waitForMessage() {


    auto self(shared_from_this());

//std::cout << "now starting \n";

            auto m_message = std::allocate_shared<DGMessage>(m_alloc);

    boost::asio::async_read(*m_socket, boost::asio::buffer(m_message.get(), sizeof(DGMessage)),
        [this, self, m_message](const boost::system::error_code& ec, std::size_t /*length*/) {
            if (!ec) {

                bool redistrib = false;
                bool excludeyou = false;

                //std::cout << "got something, visiting \n";
                visit([this, &redistrib, &excludeyou](auto& m) {
                    using T = std::decay_t<decltype(m)>;
                    if constexpr (std::is_same_v<T, WorldInfo>) {

                    }
                    else if constexpr (std::is_same_v<T, ControlsUpdate>) {
                        //std::cout << "Got controls update from " << m_playerIndex << " \n";
                        // {
                        //     //std::cout << "Waiting on clients lock \n";
                        //     std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
                        //     clients.at(m_playerIndex).camera.transform.position = m.startPos;
                        //     clients.at(m_playerIndex).controls = m.myControls;
                        //     clients.at(m_playerIndex).camera.transform.updateWithYawPitch(m.startYawPitch.x, m.startYawPitch.y);
                        //     //std::cout << "Got clients lock \n";
                        // }

                        //Generate the trees etc serverside. Turned off for now because I don't think it's worth it.
                        // if (auto g = generatedChunks.lock(); g)
                        // {
                        //     auto concerningChunkPos = TwoIntTup(m.startPos.x, m.startPos.z);
                        //     for (int x = -1; x < 2; x++)
                        //     {
                        //         for (int z = -1; z < 2; z++)
                        //         {
                        //             auto resultantspot = concerningChunkPos + TwoIntTup(x, z);
                        //             if (!g->contains(resultantspot))
                        //             {
                        //                 WorldRenderer::generateChunk(&serverWorld, resultantspot);
                        //                 g->insert(resultantspot);
                        //             }
                        //
                        //         }
                        //     }
                        // }
                        boost::asio::post(localserver_io_context->get_executor(), [m_playerIndex = m_playerIndex, m](){
                            {
                                std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
                                auto & c = serverReg.get<jl::Camera>(m_playerIndex);
                                c.transform.position = m.startPos;
                                serverReg.get<Controls>(m_playerIndex) = m.myControls;
                                c.transform.updateWithYawPitch(m.startYawPitch.x, m.startYawPitch.y);
                            }
                        });
                    
                        redistrib = true;
                        excludeyou = true;
                    }
                    else if constexpr (std::is_same_v<T, PlayerPresent>) {

                    }
                    else if constexpr (std::is_same_v<T, PlayerLeave>) {

                    }
                    else if constexpr (std::is_same_v<T, YawPitchUpdate>) {
                        //std::cout << "Got yawpitchupdate \n";
                        redistrib = true;
                        excludeyou = true;
                    }
                    else if constexpr (std::is_same_v<T, PlayerSelectBlockChange>) {
                        //std::cout << "Got selectblockchange \n";

                        boost::asio::post(localserver_io_context->get_executor(), [m_playerIndex = m_playerIndex, m](){
                            {
                                std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
                                serverReg.patch<InventoryComponent>(m_playerIndex, [&clientsLock, &m](InventoryComponent & inv)
                                {
                                    inv.currentHeldInvIndex = m.newMaterial;
                                });
                            }
                        });
                        
                        redistrib = true;
                        excludeyou = true;
                    }
                    if constexpr (std::is_same_v<T, DepleteInventorySlot> || std::is_same_v<T, BlockSetAndDepleteSlot>)
                    {

                        DepleteInventorySlot msg;

                        if constexpr( std::is_same_v<T, BlockSetAndDepleteSlot>)
                        {
                            msg = getDepleteSlot(m);
                        } else
                        {
                            msg = m;
                        }

                        boost::asio::post(localserver_io_context->get_executor(), [m_playerIndex = m_playerIndex, msg](){
                            {
                               // std::cout << "Got Deplete on server" << std::endl;
                                std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
                                serverReg.patch<InventoryComponent>(msg.playerIndex, [&msg, m_playerIndex](InventoryComponent & inv)
                                {
                                    try
                                    {
                                        auto & slot = inv.inventory.inventory.at(msg.slot);
                                        if(slot.count - msg.depletion > 0)
                                        {
                                            slot.count -= msg.depletion;
                                        } else
                                        {
                                            slot.count = 0;
                                            slot.block = 0;
                                        }

                                    } catch (std::exception&e)
                                    {
                                        std::cerr << "slot " << msg.slot << " didnt exist " << (int)m_playerIndex << " dumbass" << std::endl;
                                    }

                                });
                            }
                        });
                        
                        redistrib = true;
                    }
                    else if constexpr (std::is_same_v<T, RequestInventorySwap>)
                    {
                        // std::cout << "RequestInventorySwap: Source: " << m.sourceID << " Dest: " << m.destinationID << "\n";
                        // std::cout << "PlayerIndex: " << m.myPlayerIndex << " SrcInd: " << (int)m.sourceIndex << " DstInd: " << (int)m.destinationIndex << "\n";
                        // std::cout << "MouseSlotS: " << std::to_string(m.mouseSlotS) << " MouseSlotD: " << std::to_string(m.mouseSlotD) << "\n";

                        //Swaps can only be done within ones own inventory

                            if (m.sourceID == m.destinationID)
                            {

                                std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);

                                InventorySlot* source = nullptr;
                                InventorySlot* destination = nullptr;

                                auto view = serverReg.view<UUIDComponent, InventoryComponent>();

                                for (auto entity : view)
                                {
                                    auto & inventory = view.get<InventoryComponent>(entity);
                                    auto & uuid = view.get<UUIDComponent>(entity);

                                    if (uuid.uuid == m.destinationID)
                                    {
                                        if (m.mouseSlotD)
                                        {
                                            destination = &inventory.inventory.mouseHeldItem;
                                        }
                                        else
                                        {
                                            destination = &inventory.inventory.inventory.at(m.destinationIndex);
                                        }

                                    }
                                    if (uuid.uuid == m.sourceID)
                                    {
                                        if (m.mouseSlotS)
                                        {
                                            source = &inventory.inventory.mouseHeldItem;
                                        } else
                                        {
                                            source = &inventory.inventory.inventory.at(m.sourceIndex);
                                        }
                                    }
                                }

                                if (source && destination)
                                {
                                    InventorySlot srcCopy = *source;
                                    *source = *destination;
                                    *destination = srcCopy;
                                    redistrib = true;
                                }

                            }


                    }

                    else if constexpr (std::is_same_v<T, RequestStackSlotsToDest>)
                    {

                        std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);

                                InventorySlot* source = nullptr;
                                InventorySlot* destination = nullptr;

                                auto view = serverReg.view<UUIDComponent, InventoryComponent>();

                                for (auto entity : view)
                                {
                                    auto & inventory = view.get<InventoryComponent>(entity);
                                    auto & uuid = view.get<UUIDComponent>(entity);

                                    if (uuid.uuid == m.destinationID)
                                    {
                                        if (m.mouseSlotD)
                                        {
                                            destination = &inventory.inventory.mouseHeldItem;
                                        }
                                        else
                                        {
                                            destination = &inventory.inventory.inventory.at(m.destinationIndex);
                                        }

                                    }
                                    if (uuid.uuid == m.sourceID)
                                    {
                                        if (m.mouseSlotS)
                                        {
                                            source = &inventory.inventory.mouseHeldItem;
                                        } else
                                        {
                                            source = &inventory.inventory.inventory.at(m.sourceIndex);
                                        }
                                    }
                                }

                                if (source && destination)
                                {
                                    if (source->block == destination->block)
                                    {
                                        auto amtDestCanTake = std::min((int)source->count, 99 - destination->count);

                                        if (amtDestCanTake > 0)
                                        {
                                            destination->count += amtDestCanTake;

                                           source->count -= amtDestCanTake;
                                           if (source->count <= 0)
                                           {
                                               source->count = 0;
                                               source->block = 0;
                                           }
                                           redistrib = true;
                                        }


                                    }

                                }
                    }

                    if constexpr (std::is_same_v<T, BlockSet> || std::is_same_v<T, BlockSetAndDepleteSlot>) {

                        BlockSet msg;

                        if constexpr( std::is_same_v<T, BlockSetAndDepleteSlot>)
                        {
                            msg = getBlockSet(m);
                        } else
                        {
                            msg = m;
                        }
                        
                            msg.newEntityNameIfApplicable = entt::null;
                            auto blockThere = serverWorld.get(msg.spot);

                            if(auto func = findSpecialSetBits((MaterialName)(msg.block & BLOCK_ID_BITS)); func != std::nullopt)
                            {

                                auto campos = msg.pp;
                                func.value()(&serverWorld, msg.spot, campos);

                            } else
                            {
                                serverWorld.userDataMap.set(msg.spot, msg.block);
                            }

                            if (blockThere != AIR && (blockThere != (MaterialName)(msg.block & BLOCK_ID_BITS)))
                            {
                                if (auto f = findSpecialRemoveBits((MaterialName)blockThere); f != std::nullopt)
                                {

                                    f.value()(&serverWorld, msg.spot);
                                }
                            }

                            //Removing block entity if there

                            if(auto f = findEntityRemoveFunc((MaterialName)(blockThere)); f != std::nullopt)
                            {

                                f.value()(serverReg, msg.spot);
                            }

                            //Adding block entity
                            if(auto func = findEntityCreateFunc((MaterialName)(msg.block & BLOCK_ID_BITS)); func != std::nullopt)
                            {

                                auto newe = func.value()(serverReg, msg.spot, entt::null);
                                msg.newEntityNameIfApplicable = newe;
                            }

                        if (msg.addLootDrop != std::nullopt)
                        {
                            auto & ald = msg.addLootDrop.value();
                            //std::cout << "Adding loot drop on server at " << ald.spot.x << " " << ald.spot.y << " " << ald.spot.z << std::endl;
                            clientsMutex.lock();

                            auto newe = makeLootDrop(serverReg, ald.lootDrop, ald.spot);
                            ald.newEntityName = newe;
                            clientsMutex.unlock();


                        }

                        
                        redistrib = true;
                    }
                    else if constexpr (std::is_same_v<T, VoxModelStamp>)
                    {

                        boost::asio::post(localserver_io_context->get_executor(), [m_playerIndex =  m_playerIndex, m](){


                            auto v = PlacedVoxModel {
                            m.name, m.spot};
                            {
                                std::unique_lock<std::shared_mutex> barlock(serverWorld.placedVoxModels.mutex);
                                serverWorld.placedVoxModels.models.push_back(v);
                            }


                            auto & vm = voxelModels[m.name];
                            std::vector<IntTup> spotsToEraseInUDM;
                            spotsToEraseInUDM.reserve(500);

                            {
                                //We're gonna do the block placing, then ask main to request the rebuilds because we won't know what chunks are active when we're done.


                                std::shared_lock<std::shared_mutex> udmRL(serverWorld.userDataMap.mutex());
                                auto lock = serverWorld.nonUserDataMap.getUniqueLock();

                                IntTup offset = IntTup(vm.dimensions.x/-2, 0, vm.dimensions.z/-2) + m.spot;
                                for ( auto & p : vm.points)
                                {
                                    serverWorld.setNUDMLocked(offset+p.localSpot, p.colorIndex);
                                    if (serverWorld.userDataMap.getUnsafe(offset+p.localSpot) != std::nullopt)
                                    {
                                        spotsToEraseInUDM.emplace_back(offset+p.localSpot);
                                    }

                                }



                            }
                            {
                                auto lock = serverWorld.userDataMap.getUniqueLock();
                                for (auto & spot : spotsToEraseInUDM)
                                {
                                    serverWorld.userDataMap.erase(spot, true);
                                }
                            }

                        });

                        //auto & vm = voxelModels[(int)m.name];

                        redistrib = true;
                    }
                    else if constexpr (std::is_same_v<T, AddLootDrop>)
                    {
                        //td::cout << "Adding loot drop on server at " << m.spot.x << " " << m.spot.y << " " << m.spot.z << std::endl;
                        clientsMutex.lock();

                        auto newe = makeLootDrop(serverReg, m.lootDrop, m.spot);
                        m.newEntityName = newe;
                        clientsMutex.unlock();


                        redistrib = true;
                    }
                    else if constexpr (std::is_same_v<T, PickUpLootDrop>)
                    {
                        //We now, in the interest of freeing up this session to be always available, will not be able to decide here whether to redistrib.
                        //So we will redistrib no matter what, and do this simple validity check on the client as well
                        boost::asio::post(localserver_io_context->get_executor(), [m_playerIndex = m_playerIndex, m](){
                            clientsMutex.lock();
                            if (serverReg.valid(m.lootDrop) && serverReg.valid(m.myPlayerIndex))
                            {
                                if (serverReg.all_of<LootDrop>(m.lootDrop))
                                {
                                    auto loot = serverReg.get<LootDrop>(m.lootDrop);
                                    InventoryComponent & playerInv = serverReg.get<InventoryComponent>(m.myPlayerIndex);

                                    if (playerInv.add(loot))
                                    {
                                        serverReg.destroy(m.lootDrop);
                                    }
                                }
                            }
                            clientsMutex.unlock();
                        });
                        
                        redistrib = true;
                    }
                    else if constexpr(std::is_same_v<T, DoRecipeOnMyInv>)
                    {
                        clientsMutex.lock();
                        if (serverReg.valid(m.myPlayerIndex))
                        {
                            if (serverReg.all_of<InventoryComponent>(m.myPlayerIndex))
                            {
                                auto& inv = serverReg.get<InventoryComponent>(m.myPlayerIndex);
                                doRecipeOnInv(inv, m.recipeIndex);
                            }
                        }
                        clientsMutex.unlock();
                        redistrib = true;
                    }
                    else if constexpr (std::is_same_v<T, BulkBlockSet>) {
                        boost::asio::post(localserver_io_context->get_executor(), [m](){

                            auto b = BlockArea{m.corner1, m.corner2, m.block, m.hollow};

                            {
                                std::unique_lock<std::shared_mutex> barlock(serverWorld.blockAreas.baMutex);
                                serverWorld.blockAreas.blockAreas.push_back(b);
                                // if (b.hollow)
                                // {
                                //     serverWorld.blockAreas.blockAreas.push_back(BlockArea{
                                //     m.corner1 + IntTup(1,1,1), m.corner2 + IntTup(-1,-1,-1), AIR, false});
                                // }
                            }

                            {
                                std::vector<IntTup> spotsToEraseInUDM;
                                spotsToEraseInUDM.reserve(500);


                                {

                                    //             std::unordered_set<TwoIntTup, TwoIntTupHash> implicatedChunks;
                                    // auto lock = world->nonUserDataMap.getUniqueLock();
                                    // std::shared_lock<std::shared_mutex> udmRL(world->userDataMap.mutex());
                                    // auto m = request.area;
                                    // int minX = std::min(m.corner1.x, m.corner2.x);
                                    // int maxX = std::max(m.corner1.x, m.corner2.x);
                                    // int minY = std::min(m.corner1.y, m.corner2.y);
                                    // int maxY = std::max(m.corner1.y, m.corner2.y);
                                    // int minZ = std::min(m.corner1.z, m.corner2.z);
                                    // int maxZ = std::max(m.corner1.z, m.corner2.z);
                                    //
                                    // for (int x = minX; x <= maxX; x++) {
                                    //     for (int y = minY; y <= maxY; y++) {
                                    //         for (int z = minZ; z <= maxZ; z++) {
                                    //             bool isBoundary = (x == minX || x == maxX ||
                                    //                 y == minY || y == maxY ||
                                    //                 z == minZ || z == maxZ);
                                    //
                                    //             if (isBoundary || !m.hollow) {
                                    //                 world->setNUDMLocked(IntTup{x, y, z}, m.block);
                                    //                 if (world->userDataMap.getUnsafe(IntTup{x, y, z}) != std::nullopt)
                                    //                 {
                                    //                     spotsToEraseInUDM.emplace_back(x, y, z);
                                    //                 }
                                    //             }
                                    //
                                    //         }
                                    //     }
                                    // }

                                    std::shared_lock<std::shared_mutex> udmRL(serverWorld.userDataMap.mutex());
                                        auto lock = serverWorld.nonUserDataMap.getUniqueLock();
                                    auto hulk = b;
                                    int minX = std::min(hulk.corner1.x, hulk.corner2.x);
                                    int maxX = std::max(hulk.corner1.x, hulk.corner2.x);
                                    int minY = std::min(hulk.corner1.y, hulk.corner2.y);
                                    int maxY = std::max(hulk.corner1.y, hulk.corner2.y);
                                    int minZ = std::min(hulk.corner1.z, hulk.corner2.z);
                                    int maxZ = std::max(hulk.corner1.z, hulk.corner2.z);

                                    for (int x = minX; x <= maxX; x++) {
                                        for (int y = minY; y <= maxY; y++) {
                                            for (int z = minZ; z <= maxZ; z++) {

                                                bool isBoundary = (x == minX || x == maxX ||
                                                            y == minY || y == maxY ||
                                                            z == minZ || z == maxZ);

                                                if (isBoundary || !b.hollow)
                                                {
                                                    serverWorld.setNUDMLocked(IntTup{x, y, z}, m.block);
                                                    if (serverWorld.userDataMap.getUnsafe(IntTup{x, y, z}) != std::nullopt)
                                                    {
                                                        spotsToEraseInUDM.emplace_back(IntTup{x, y, z});
                                                    }
                                                }

                                            }
                                        }
                                    }
                                }

                                {
                                    auto lock = serverWorld.userDataMap.getUniqueLock();
                                    for (auto & spot : spotsToEraseInUDM)
                                    {
                                        serverWorld.userDataMap.erase(spot, true);
                                    }
                                }

                            }

                        });

                        redistrib = true;
                    }
                    else if constexpr (std::is_same_v<T, FileTransferInit>) {


                    }
                }, *m_message);


                if (redistrib)
                {
                    //boost::asio::post(localserver_io_context->get_executor(), [&, m_playerIndex = m_playerIndex](){
                        sendMessageToAllClients(*m_message, m_playerIndex, excludeyou);
                   // });
                   
                }

                //Wait for next message
            waitForMessage();
            } else {

                if (ec == boost::asio::error::connection_reset ||
                    ec == boost::asio::error::eof ||
                    ec == boost::asio::error::operation_aborted ||
                    ec == boost::asio::error::connection_aborted ||
                    ec == boost::asio::error::not_connected ||
                    ec == boost::asio::error::broken_pipe ||
                    ec == boost::asio::error::shut_down)
                {



                    DGMessage pl = PlayerLeave {
                                    m_playerIndex};

                    // {
                    //     std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
                    //     //std::cout << "New player count: " << clients.size()-1 << std::endl;
                    //     //serverReg.erase(m_playerIndex);
                    // }

                    sendMessageToAllClients(pl, m_playerIndex, true);

                    //Save when a player leaves too
                    saveDM("serverworld.txt", &serverWorld.userDataMap, serverWorld.blockAreas, serverWorld.placedVoxModels, invMapKeyedByUID, serverReg, "serversnap.bin");
                }
            }
        });


}

private:
    std::shared_ptr<tcp::socket> m_socket;

    std::vector<char> m_body;       // To store the raw message body
    entt::entity m_playerIndex;
    ClientUID m_clientUID;
};

class Server {
public:

    FlatCposSetPoolAllocator<TwoIntTup> alloc = {gcspool.get()};

    std::shared_ptr<flatCposSet> generatedChunks = std::make_shared<flatCposSet>();





    Server(boost::asio::io_context& io_context, short port) : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
        serverWorld.setSeed(DGSEEDSEED);

        loadDM("world/serverworld.txt", &serverWorld, serverReg, serverWorld.blockAreas, serverWorld.placedVoxModels, nullptr, nullptr, "world/savedReg.bin");
        auto g = gcspool.get();
        if (serverReg.view<WorldState>().empty())
        {
            auto e= serverReg.create();
            serverReg.emplace<WorldState>(e, WorldState::BEGINNING);
        }
        // now we call do_accept() where we wait for clients

        boost::asio::post(localserver_thread_pool->get_executor(), [](){
            thread_local float TOD = 300.0f;
            while (localserver_running.load())
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                {
                    std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
                    const auto view = serverReg.view<PlayerComp, NetworkComponent>();
                    for (auto entity : view)
                    {
                        auto& nwc = view.get<NetworkComponent>(entity);
                        DGMessage hb = HeartbeatAndCleanup{TOD};
                        if (!nwc.socket.expired())
                        {
                            boost::asio::write(*nwc.socket.lock(), boost::asio::buffer(&hb, sizeof(DGMessage)));
                        }
                    }
                    updateOrDestroyLifetimeHavers(serverReg, 0);
                    TOD = std::fmod(TOD + 1.0f, DAY_LENGTH);
                }
            }


        });

        do_accept();
    }

    ~Server()
    {

        saveDM("world/serverworld.txt", &serverWorld.userDataMap, serverWorld.blockAreas, serverWorld.placedVoxModels, invMapKeyedByUID, serverReg, "world/savedReg.bin");
        serverWorld.userDataMap.clear();
        serverWorld.blockAreas.baMutex.lock();
        serverWorld.blockAreas.blockAreas.clear();
        serverWorld.blockAreas.baMutex.unlock();
        clientsMutex.lock();
        serverReg.clear();

        clientsMutex.unlock();
    }

    Server(const Server& other) = delete;
    Server(Server&& other) = delete;
    Server& operator=(const Server& other) = delete;
    Server& operator=(Server&& other) = delete;


    void do_accept() {
        // this is an async accept which means the lambda function is
        // executed, when a client connects
        m_acceptor.async_accept([this](const boost::system::error_code& ec, tcp::socket socket) {

            if (!ec) {
                // Create a shared_ptr for the socket


                try
                {
                    auto shared_socket = std::make_shared<tcp::socket>(std::move(socket));

                    // Log the connection


                    // auto index = serverReg.create();
                    // //emplacePlayerParts(serverReg, index);
                    //
                    // // static int ind = 0;
                    // // auto index = ind++;
                    //
                    // {
                    //     std::shared_lock<std::shared_mutex> clientslock(clientsMutex);
                    //
                    //     clients.insert({index, ServerPlayer{
                    //         .socket = shared_socket,
                    //         .controls = Controls{},
                    //         .camera = jl::Camera{},
                    //         .receivedWorld = std::atomic<bool>{false},
                    //     }});
                    // }

                    // Pass the shared_ptr to the session
                    auto sesh = std::make_shared<Session>(shared_socket, generatedChunks);
                     sesh->run();
                } catch (std::exception& e)
                {

                }



            } else {

            }

            do_accept();
        });
    }

private:
    tcp::acceptor m_acceptor;
};

inline void serverThreadFun(int port)
{
    boost::asio::io_context io_context;
    Server s(io_context, port);
    io_context.run();
}


inline void localServerThreadFun(int port)
{

    if (!localserver_io_context) {
        localserver_io_context = std::make_unique<boost::asio::io_context>();

    }
    if (!localserver_thread_pool)
    {
        localserver_thread_pool = std::make_unique<boost::asio::thread_pool>(1);
    }
    localserver_io_context->restart();
    Server s(*localserver_io_context, port);
    localserver_io_context->run();
    localserver_running.store(false);
    localserver_thread_pool->join();
}

inline void launchLocalServer(int port)
{

    if (!localserver_running.load()) {

        if (!localserver_thread_pool)
        {
            localserver_thread_pool = std::make_unique<boost::asio::thread_pool>(1);
        }
        if (!localserver_io_context) {
            localserver_io_context = std::make_unique<boost::asio::io_context>();

        } else {
            localserver_io_context->restart();
        }
        localserver_running.store(true);
        localserver_thread = std::thread(localServerThreadFun, port);


        localserver_io_context.reset(new boost::asio::io_context());

    } else {

    }
}
inline void endLocalServerIfRunning()
{
    if (localserver_running.load())
    {
        localserver_running.store(false);
        localserver_io_context->stop();

        localserver_thread.join();


    }

}


inline void launchServerThreadFun()
{
    std::thread serverThread(serverThreadFun, 6969);
    serverThread.detach();
}

#endif //SERVER_H