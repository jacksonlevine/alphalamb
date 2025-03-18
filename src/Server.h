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
#include "components/PlayerEmplacer.h"
#include "components/UUIDComponent.h"
#include "specialblocks/FindSpecialBlock.h"
#include "world/DataMap.h"
#include "world/World.h"
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
        std::cout << "Error opening file for binary: " << filename << std::endl;
    }
    return std::vector<char>();
}

class Session : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(std::shared_ptr<tcp::socket> socket)
    : m_socket(std::move(socket)), m_playerIndex(entt::null) { }

    void run() {
        sayInitialThings();
    }
private:

    void sayInitialThings()
    {
        auto self(shared_from_this());


        //Client needs to send their greeting before they get anything.
        ClientToServerGreeting playerInit = {};
        boost::asio::read(*m_socket, boost::asio::buffer(&playerInit, sizeof(DGMessage)));
        std::cout << "Server got client UID: " << playerInit.id << std::endl;
        m_clientUID = playerInit.id;

        {
            std::shared_lock<std::shared_mutex> clientslock(clientsMutex);


            auto view = serverReg.view<UUIDComponent>();
            for (auto entity : view)
            {
                auto comp = view.get<UUIDComponent>(entity);
                if (m_clientUID == comp.uuid)
                {
                    std::cout << "Player already exists in registry! " << m_clientUID << std::endl;
                    m_playerIndex = entity;
                    break;
                }

            }
            if (m_playerIndex == entt::null)
            {
                std::cout << "Player is new to this registry. " << m_clientUID << std::endl;
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



        DGMessage wi = WorldInfo {
            .seed = DGSEEDSEED,
            .yourPosition = glm::vec3(0, 200, 0),
            .yourPlayerIndex = m_playerIndex

        };

        boost::asio::write(*m_socket, boost::asio::buffer(&wi, sizeof(DGMessage)));



        //
        // {
        //     std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
        //     clients.at(m_playerIndex).myUID = m_clientUID;
        // }

        //Now the players inv will exist in the world they download

        auto string = saveDM("serverworld.txt", serverWorld.userDataMap, serverWorld.blockAreas, serverWorld.placedVoxModels, invMapKeyedByUID, serverReg, "serversnap.bin");
        if (string.has_value())
        {

            auto regfile = loadBinaryFile("serversnap.bin");

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
                    boost::asio::async_write(*m_socket, boost::asio::buffer(&playerPresent, sizeof(DGMessage)),
                    [this, self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred)
                    {
                        if (!ec) {
                            std::cout << "Successfully wrote playerpresent " << bytes_transferred << " bytes." << std::endl;
                        } else {
                            std::cerr << "Error writing to socket: " << ec.message() << std::endl;
                        }
                    });
                    //And also notify this person we are here
                    if(!nc.socket.expired() && nc.receivedWorld)
                    {
                        DGMessage ourPlayerPresent = PlayerPresent {
                            .index = m_playerIndex, .position = serverReg.get<jl::Camera>(m_playerIndex).transform.position,
                            .direction = serverReg.get<jl::Camera>(m_playerIndex).transform.direction, .id = m_clientUID
                        };

                        boost::asio::async_write(*nc.socket.lock(), boost::asio::buffer(&ourPlayerPresent, sizeof(DGMessage)),
                    [this, self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred)
                        {
                            if (!ec) {
                                std::cout << "Successfully wrote ourplayerpresent " << bytes_transferred << " bytes." << std::endl;
                            } else {
                                std::cerr << "Error writing to socket: " << ec.message() << std::endl;
                            }
                        });
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


    boost::asio::async_read(*m_socket, boost::asio::buffer(&m_message, sizeof(DGMessage)),
        [this, self](const boost::system::error_code& ec, std::size_t /*length*/) {
            if (!ec) {

                bool redistrib = false;
                bool excludeyou = false;

                //std::cout << "got something, visiting \n";
                visit([&](const auto& m) {
                    using T = std::decay_t<decltype(m)>;
                    if constexpr (std::is_same_v<T, WorldInfo>) {
                        std::cout << "Got world info " << m.seed << " \n";
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

                        {
                            std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
                            auto & c = serverReg.get<jl::Camera>(m_playerIndex);
                            c.transform.position = m.startPos;
                            serverReg.get<Controls>(m_playerIndex) = m.myControls;
                            c.transform.updateWithYawPitch(m.startYawPitch.x, m.startYawPitch.y);
                        }

                        redistrib = true;
                        excludeyou = true;
                    }
                    else if constexpr (std::is_same_v<T, PlayerPresent>) {
                        std::cout << "Got playerpresent \n";
                    }
                    else if constexpr (std::is_same_v<T, PlayerLeave>) {
                        std::cout << "Got playerleave \n";
                    }
                    else if constexpr (std::is_same_v<T, YawPitchUpdate>) {
                        //std::cout << "Got yawpitchupdate \n";
                        redistrib = true;
                        excludeyou = true;
                    }
                    else if constexpr (std::is_same_v<T, PlayerSelectBlockChange>) {
                        //std::cout << "Got selectblockchange \n";
                        redistrib = true;
                        excludeyou = true;
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
                                auto inventory = view.get<InventoryComponent>(entity);
                                auto uuid = view.get<UUIDComponent>(entity);

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
                    else if constexpr (std::is_same_v<T, BlockSet>) {
                        std::cout << "Got block set" << m.block << "\n";
                        if(auto func = findSpecialSetBits((MaterialName)(m.block & BLOCK_ID_BITS)); func != std::nullopt)
                        {
                            std::cout << "Calling custom func on server \n";
                            func.value()(&serverWorld, m.spot);

                        } else
                        {
                            serverWorld.userDataMap->set(m.spot, m.block);
                        }

                        redistrib = true;
                    }
                    else if constexpr (std::is_same_v<T, VoxModelStamp>)
                    {
                        auto v = PlacedVoxModel {
                        m.name, m.spot};
                        {
                            std::unique_lock<std::shared_mutex> barlock(serverWorld.placedVoxModels.mutex);
                            serverWorld.placedVoxModels.models.push_back(v);
                        }

                        //auto & vm = voxelModels[(int)m.name];

                        redistrib = true;
                    }
                    else if constexpr (std::is_same_v<T, BulkBlockSet>) {
                        auto b = BlockArea{m.corner1, m.corner2, m.block, m.hollow
                        };
                        std::cout << "Got bulk block set \n";
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

                               std::shared_lock<std::shared_mutex> udmRL(serverWorld.userDataMap->mutex());
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
                                               if (serverWorld.userDataMap->getLocked(IntTup{x, y, z}) != std::nullopt)
                                               {
                                                   spotsToEraseInUDM.emplace_back(IntTup{x, y, z});
                                               }
                                           }

                                       }
                                   }
                               }
                           }

                           {
                               auto lock = serverWorld.userDataMap->getUniqueLock();
                               for (auto & spot : spotsToEraseInUDM)
                               {
                                   serverWorld.userDataMap->erase(spot, true);
                               }
                           }

                        }

                        redistrib = true;
                    }
                    else if constexpr (std::is_same_v<T, FileTransferInit>) {
                        std::cout << "Got file transfer init \n";

                    }
                }, m_message);


                if (redistrib)
                {
                   sendMessageToAllClients(m_message, m_playerIndex, excludeyou);
                }

                //Wait for next message
            waitForMessage();
            } else {
                std::cout << "Error reading message: " << ec.message() << std::endl;
                if (ec == boost::asio::error::connection_reset ||
                    ec == boost::asio::error::eof ||
                    ec == boost::asio::error::operation_aborted ||
                    ec == boost::asio::error::connection_aborted ||
                    ec == boost::asio::error::not_connected ||
                    ec == boost::asio::error::broken_pipe ||
                    ec == boost::asio::error::shut_down)
                {
                    std::cout << "Removing player \n";


                    DGMessage pl = PlayerLeave {
                                    m_playerIndex};

                    // {
                    //     std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
                    //     //std::cout << "New player count: " << clients.size()-1 << std::endl;
                    //     //serverReg.erase(m_playerIndex);
                    // }

                    sendMessageToAllClients(pl, m_playerIndex, true);

                    //Save when a player leaves too
                    saveDM("serverworld.txt", serverWorld.userDataMap, serverWorld.blockAreas, serverWorld.placedVoxModels, invMapKeyedByUID, serverReg, "serversnap.bin");
                }
            }
        });


}

private:
    std::shared_ptr<tcp::socket> m_socket;

    DGMessage m_message{};         // To store the header
    std::vector<char> m_body;       // To store the raw message body
    entt::entity m_playerIndex;
    ClientUID m_clientUID;
};

class Server {
public:
    Server(boost::asio::io_context& io_context, short port) : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
        loadRegistry(serverReg, "savedreg.bin");
        // now we call do_accept() where we wait for clients
        serverWorld.setSeed(DGSEEDSEED);
        do_accept();
    }

    void do_accept() {
        // this is an async accept which means the lambda function is
        // executed, when a client connects
        m_acceptor.async_accept([this](const boost::system::error_code& ec, tcp::socket socket) {
            std::cout << "Async accept callback called \n";
            if (!ec) {
                // Create a shared_ptr for the socket
                std::cout << "No ec \n";

                try
                {
                    auto shared_socket = std::make_shared<tcp::socket>(std::move(socket));

                    // Log the connection
                    std::cout << "creating session on: "
                              << shared_socket->remote_endpoint().address().to_string()
                              << ":" << shared_socket->remote_endpoint().port() << '\n';

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
                    auto sesh = std::make_shared<Session>(shared_socket);
                     sesh->run();
                } catch (std::exception& e)
                {
                    std::cout << "Couldn't create session: " << e.what() << "\n";
                }


                std::cout << "session created\n";
            } else {
                std::cout << "error: " << ec.message() << std::endl;
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
    localserver_io_context->restart();
    Server s(*localserver_io_context, port);
    localserver_io_context->run();
}

inline void launchLocalServer(int port)
{
    loadDM("serverworld.txt", serverWorld.userDataMap, serverReg, serverWorld.blockAreas, serverWorld.placedVoxModels);

    if (!localserver_running.load()) {
        std::cout << "Starting local server...\n";

        if (!localserver_io_context) {
            localserver_io_context = std::make_unique<boost::asio::io_context>();
        } else {
            localserver_io_context->restart();
        }

        localserver_thread = std::thread(localServerThreadFun, port);
        localserver_running.store(true);

        localserver_io_context.reset(new boost::asio::io_context());

    } else {
        std::cout << "Local server is already running.\n";
    }
}
inline void endLocalServerIfRunning()
{
    if (localserver_running.load())
    {
        localserver_io_context->stop();

        localserver_thread.join();
        localserver_running.store(false);

    }
    serverWorld.userDataMap->clear();
    serverWorld.blockAreas.baMutex.lock();
    serverWorld.blockAreas.blockAreas.clear();
    serverWorld.blockAreas.baMutex.unlock();
    saveRegistry(serverReg, "savedreg.bin");
    clientsMutex.lock();
    serverReg.clear();
    clientsMutex.unlock();
}


inline void launchServerThreadFun()
{
    std::thread serverThread(serverThreadFun, 6969);
    serverThread.detach();
}

#endif //SERVER_H