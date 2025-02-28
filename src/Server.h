//
// Created by jack on 2/9/2025.
//

#ifndef SERVER_H
#define SERVER_H

#include "Camera.h"
#include "LocalServerIOContext.h"
#include "PrecompHeader.h"
#include "Network.h"
#include "PlayerInfoMapKeyedByUID.h"
#include "world/DataMap.h"
#include "world/World.h"
using tcp = boost::asio::ip::tcp;

struct ServerPlayer
{
    std::weak_ptr<tcp::socket> socket;
    Controls controls;
    jl::Camera camera;
    bool receivedWorld = false;
};

extern std::unordered_map<int, ServerPlayer> clients;

extern std::shared_mutex clientsMutex;
//
// extern BlockAreaRegistry serverWorld.blockAreas;
// extern PlacedVoxModelRegistry serverWorld.placedVoxModels;
// extern DataMap* serverWorld.userDataMap;
extern World serverWorld;
extern InvMapKeyedByUID invMapKeyedByUID;


inline void sendMessageToAllClients(const DGMessage& message, int m_playerIndex, bool excludeMe)
{

    std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
    for(auto & [id, client] : clients)
    {
        if(!((id == (int)m_playerIndex) && excludeMe) && !client.socket.expired() && client.receivedWorld)
        {
            boost::asio::write(*client.socket.lock(), boost::asio::buffer(&message, sizeof(DGMessage)));
        }
    }

}


class Session : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(std::shared_ptr<tcp::socket> socket, size_t playerIndex)
    : m_socket(std::move(socket)), m_playerIndex(playerIndex) { }

    void run() {
        sayInitialThings();
    }
private:

    void sayInitialThings()
    {
        auto self(shared_from_this());
        constexpr int seed = 0;
        DGMessage wi = WorldInfo {
            .seed = seed,
            .yourPosition = glm::vec3(0, 200, 0),
            .yourPlayerIndex = (int)m_playerIndex

        };

        boost::asio::async_write(*m_socket, boost::asio::buffer(&wi, sizeof(DGMessage)),
        [this, self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred)
        {
            if (!ec) {
                std::cout << "Successfully wrote " << bytes_transferred << " bytes." << std::endl;
            } else {
                std::cerr << "Error writing to socket: " << ec.message() << std::endl;
            }
        });

        auto string = saveDM("serverworld.txt", serverWorld.userDataMap, serverWorld.blockAreas, serverWorld.placedVoxModels, invMapKeyedByUID);
        if (string.has_value())
        {
            DGMessage fileInit = FileTransferInit {
            .fileSize = string.value().size() * sizeof(char),
            .isWorld  = true};
            boost::asio::async_write(*m_socket, boost::asio::buffer(&fileInit, sizeof(DGMessage)),
            [this, self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred)
            {
                if (!ec) {
                    std::cout << "Successfully wrote fileinit " << bytes_transferred << " bytes." << std::endl;
                } else {
                    std::cerr << "Error writing to socket: " << ec.message() << std::endl;
                }
            });

            boost::asio::async_write(*m_socket, boost::asio::buffer(string.value().data(), string.value().size() * sizeof(char)),
            [this, self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred)
            {
                if (!ec) {
                    std::cout << "Successfully wrote world " << bytes_transferred << " bytes." << std::endl;
                } else {
                    std::cerr << "Error writing to socket: " << ec.message() << std::endl;
                }
            });
        }
        {
            std::shared_lock<std::shared_mutex> clientsLock(clientsMutex);
            for(auto & [id, client] : clients)
            {
                if(id != (int)m_playerIndex)
                {
                    DGMessage playerPresent = PlayerPresent {
                        id, client.camera.transform.position, client.camera.transform.direction};
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
                    if(!client.socket.expired() && client.receivedWorld)
                    {
                        DGMessage ourPlayerPresent = PlayerPresent {
                            (int)m_playerIndex, clients.at(m_playerIndex).camera.transform.position, clients.at(m_playerIndex).camera.transform.direction};

                        boost::asio::async_write(*client.socket.lock(), boost::asio::buffer(&ourPlayerPresent, sizeof(DGMessage)),
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
            clients.at(m_playerIndex).receivedWorld = true;
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
                        {
                            //std::cout << "Waiting on clients lock \n";
                            std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
                            clients.at(m_playerIndex).camera.transform.position = m.startPos;
                            clients.at(m_playerIndex).controls = m.myControls;
                            clients.at(m_playerIndex).camera.transform.updateWithYawPitch(m.startYawPitch.x, m.startYawPitch.y);
                            //std::cout << "Got clients lock \n";
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
                    else if constexpr (std::is_same_v<T, BlockSet>) {
                        //std::cout << "Got block set \n";
                        serverWorld.userDataMap->set(m.spot, m.block);
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
                                    (int)m_playerIndex};

                    {
                        std::unique_lock<std::shared_mutex> clientsLock(clientsMutex);
                        std::cout << "New player count: " << clients.size()-1 << std::endl;
                        clients.erase(m_playerIndex);
                    }

                    sendMessageToAllClients(pl, m_playerIndex, true);

                    //Save when a player leaves too
                    saveDM("serverworld.txt", serverWorld.userDataMap, serverWorld.blockAreas, serverWorld.placedVoxModels, invMapKeyedByUID);
                }
            }
        });


}

private:
    std::shared_ptr<tcp::socket> m_socket;

    DGMessage m_message{};         // To store the header
    std::vector<char> m_body;       // To store the raw message body
    size_t m_playerIndex;
};

class Server {
public:
    Server(boost::asio::io_context& io_context, short port) : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
        // now we call do_accept() where we wait for clients
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
                    int index = 0;
                    {
                        std::shared_lock<std::shared_mutex> clientslock(clientsMutex);

                        while (clients.contains(index))
                        {
                            index++;
                        }
                        clients.insert({index, ServerPlayer{
                            .socket = shared_socket,
                            .controls = Controls{},
                            .camera = jl::Camera{},
                            .receivedWorld = std::atomic<bool>{false},
                        }});
                    }

                    // Pass the shared_ptr to the session
                    auto sesh = std::make_shared<Session>(shared_socket, index);
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
    loadDM("serverworld.txt", serverWorld.userDataMap, serverWorld.blockAreas, serverWorld.placedVoxModels);

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
    clientsMutex.lock();
    clients.clear();
    clientsMutex.unlock();
}


inline void launchServerThreadFun()
{
    std::thread serverThread(serverThreadFun, 6969);
    serverThread.detach();
}

#endif //SERVER_H