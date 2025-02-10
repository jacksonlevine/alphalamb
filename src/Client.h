//
// Created by jack on 2/9/2025.
//

#ifndef CLIENT_H
#define CLIENT_H

#include "Network.h"
#include "PrecompHeader.h"
#include "Scene.h"

using tcp = boost::asio::ip::tcp;

inline void read_from_server(tcp::socket* socket, std::atomic<bool>* shouldRun) {
    try {
        while (shouldRun->load()) {
            DGMessage message{};
            boost::system::error_code ec;

            //Read the message
            boost::asio::read(*socket, boost::asio::buffer(&message, sizeof(DGMessage)), ec);
            if (!ec) {

                std::cout << " Got somethng \n";

                visit([&](const auto& m) {
                    using T = std::decay_t<decltype(m)>;
                    if constexpr (std::is_same_v<T, WorldInfo>) {
                        std::cout << "Got world info " << m.seed << " \n"
                        << "playerIndex: " << m.yourPlayerIndex << " \n"
                        << "yourPosition: " << m.yourPosition.x << " " << m.yourPosition.y << " " << m.yourPosition.z << " \n";

                        theScene.world->setSeed(m.seed);
                        theScene.myPlayerIndex = theScene.addPlayerWithIndex(m.yourPlayerIndex);

                        //Dont do this yet, receive the file first
                        //theScene.worldReceived = true;
                    }
                    else if constexpr (std::is_same_v<T, ControlsUpdate>) {
                        std::cout << "Got controls update \n";
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
                                file.write(filemsg, m.fileSize);
                                file.close();

                                if(isWorld)
                                {
                                    theScene.world->load("mpworld.txt");
                                    theScene.worldReceived.store(true);
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

#endif //CLIENT_H
