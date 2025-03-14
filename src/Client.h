//
// Created by jack on 2/9/2025.
//

#ifndef CLIENT_H
#define CLIENT_H

#include "NetworkTypes.h"
#include "PrecompHeader.h"
#include "Scene.h"

using tcp = boost::asio::ip::tcp;

struct BlockChange
{
    IntTup spot;
    BlockType block;
};
extern boost::lockfree::spsc_queue<DGMessage, boost::lockfree::capacity<512>> networkToMainBlockChangeQueue;

extern boost::lockfree::spsc_queue<DGMessage, boost::lockfree::capacity<512>> mainToNetworkBlockChangeQueue;

extern std::mutex networkMutex;
extern std::condition_variable networkCV;

inline void sendToServer(tcp::socket* socket, std::atomic<bool>* shouldRun)
{
    while (shouldRun->load()) {

        {
            std::unique_lock<std::mutex> lock(networkMutex);
            if (mainToNetworkBlockChangeQueue.empty()) {

                networkCV.wait(lock);
            }
        }


        DGMessage m = WorldInfo{};
        while (mainToNetworkBlockChangeQueue.pop(&m))
        {
            boost::asio::write(*socket, boost::asio::buffer(&m, sizeof(DGMessage)));
        }
    }
}

inline void pushToMainToNetworkQueue(const DGMessage& m)
{
    if (mainToNetworkBlockChangeQueue.write_available() > 0)
    {
        mainToNetworkBlockChangeQueue.push(m);
        networkCV.notify_one();
    } else
    {
        std::cout << "No space available in main to network queue \n";
    }

}

inline void pushToNetworkToMainQueue(const DGMessage& m)
{
    if (networkToMainBlockChangeQueue.write_available() > 0)
    {
        networkToMainBlockChangeQueue.push(m);
        //networkCV.notify_one();
    } else
    {
        std::cout << "No space available in network to main queue \n";
    }
}

inline void read_from_server(tcp::socket* socket, std::atomic<bool>* shouldRun) {
    try {
        while (shouldRun->load()) {
            DGMessage message{};
            boost::system::error_code ec;

            //Read the message
            boost::asio::read(*socket, boost::asio::buffer(&message, sizeof(DGMessage)), ec);
            if (!ec) {

                //std::cout << " Got somethng \n";

                visit([&](const auto& m) {
                    using T = std::decay_t<decltype(m)>;
                    if constexpr (std::is_same_v<T, WorldInfo>) {
                        std::cout << "Got world info " << m.seed << " \n"
                        << "playerIndex: " << m.yourPlayerIndex << " \n"
                        << "yourPosition: " << m.yourPosition.x << " " << m.yourPosition.y << " " << m.yourPosition.z << " \n";

                        theScene.world->setSeed(m.seed);
                        theScene.myPlayerIndex = theScene.addPlayerWithIndex(m.yourPlayerIndex);

                        DGMessage msg = ClientToServerGreeting {
                        .id = theScene.settings.clientUID};
                        boost::system::error_code ec;
                        boost::asio::write(*socket, boost::asio::buffer(&msg, sizeof(DGMessage)), ec);
                        //Dont do this yet, receive the file first
                        //theScene.worldReceived = true;
                    }
                    else if constexpr (std::is_same_v<T, ControlsUpdate>) {
                        //std::cout << "Got controls update \n";
                        pushToNetworkToMainQueue(m);
                    }
                    else if constexpr (std::is_same_v<T, PlayerSelectBlockChange>) {
                        //std::cout << "Got select block change update \n";
                        pushToNetworkToMainQueue(m);
                    }
                    else if constexpr (std::is_same_v<T, VoxModelStamp>)
                    {
                        pushToNetworkToMainQueue(m);
                    }
                    else if constexpr (std::is_same_v<T, PlayerLeave>) {
                        std::cout << "Got player leave \n";
                        pushToNetworkToMainQueue(m);
                    }
                    else if constexpr (std::is_same_v<T, BulkBlockSet>) {
                        std::cout << "Got bulk block set \n";
                        pushToNetworkToMainQueue(m);
                    }
                    else if constexpr (std::is_same_v<T, YawPitchUpdate>) {
                        //std::cout << "Got yawpitch update \n";
                        pushToNetworkToMainQueue(m);
                    }
                    else if constexpr (std::is_same_v<T, PlayerPresent>) {
                       std::cout << "Got player present \n";
                        pushToNetworkToMainQueue(m);
                    }
                    else if constexpr (std::is_same_v<T, BlockSet>) {
                        //std::cout << "Got block set \n";
                        pushToNetworkToMainQueue(m);
                    }
                    else if constexpr (std::is_same_v<T, RequestInventorySwap>)
                    {
                        pushToNetworkToMainQueue(m);
                    }
                    else if constexpr (std::is_same_v<T, FileTransferInit>) {
                        std::cout << "Got file transfer init \n";

                        std::vector<char> filemsg(m.fileSize);
                        boost::system::error_code ec;

                        bool isWorld = m.isWorld;

                        //Read the message
                        boost::asio::read(*socket, boost::asio::buffer(filemsg.data(), m.fileSize), ec);
                        if(!ec)
                        {
                            std::cout << "Got world file \n";
                            std::ofstream file("mpworld.txt", std::ios::trunc);
                            if(file.is_open())
                            {
                                file.write(filemsg.data(), m.fileSize);
                                file.close();

                                if(isWorld)
                                {
                                    theScene.world->load("mpworld.txt", theScene.existingInvs);
                                    std::cout << "Playerindex here: " << theScene.myPlayerIndex << " \n";
                                    theScene.worldReceived.store(true);
                                    if (theScene.existingInvs.contains(theScene.settings.clientUID))
                                    {
                                        if (auto inv = loadInvFromFile("mpworld.txt", theScene.settings.clientUID))
                                        {
                                            theScene.players.at(theScene.myPlayerIndex)->inventory = inv.value();
                                        }
                                    }

                                }

                            } else
                            {
                                std::cout << "Couldn't open mpworld.txt to write \n";
                            }
                        }
                    }
                }, message);


            } else
            {
                std::cerr << "Error reading message: " << ec.message() << std::endl;
            }






        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in read_from_server: " << e.what() << std::endl;
    }
}

inline void launchReceiverThread(tcp::socket* socket, std::atomic<bool>* shouldRun)
{
    std::thread receiver_thread(read_from_server, socket, shouldRun);
    receiver_thread.detach();
}

inline void launchSenderThread(tcp::socket* socket, std::atomic<bool>* shouldRun)
{
    std::thread senderthread(sendToServer, socket, shouldRun);
    senderthread.detach();
}

#endif //CLIENT_H
