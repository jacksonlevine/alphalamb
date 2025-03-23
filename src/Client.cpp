//
// Created by jack on 2/9/2025.
//

#include "Client.h"
boost::lockfree::spsc_queue<DGMessage, boost::lockfree::capacity<512>> networkToMainBlockChangeQueue;

boost::lockfree::spsc_queue<DGMessage, boost::lockfree::capacity<512>> mainToNetworkBlockChangeQueue;


std::mutex networkMutex = {};
std::condition_variable networkCV = {};

void read_from_server(tcp::socket* socket, std::atomic<bool>* shouldRun)
{
    try {

        DGMessage msg = ClientToServerGreeting {
            .id = theScene.settings.clientUID};
        boost::system::error_code ec;
        boost::asio::write(*socket, boost::asio::buffer(&msg, sizeof(DGMessage)), ec);

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
                            << "playerIndex: " << (int)m.yourPlayerIndex << " \n"
                            << "yourPosition: " << m.yourPosition.x << " " << m.yourPosition.y << " " << m.yourPosition.z << " \n";

                        theScene.world->setSeed(m.seed);
                        //theScene.myPlayerIndex = theScene.addPlayerWithIndex(m.yourPlayerIndex, theScene.settings.clientUID);
                        auto pos = theScene.REG.get<jl::Camera>(theScene.myPlayerIndex).transform.position;
                        auto width = 0, height = 0;
                        //Get our cam ready before the first player update becasue of the "intro screen thing"
                        glfwGetWindowSize(theScene.window, &width, &height);
                        theScene.REG.get<jl::Camera>(theScene.myPlayerIndex).updateProjection(width, height, 90.0f);

                        //We could restore the cameras real stored rotatoin here by calling with the camera pitch & yaw but then the planet visual won't be facing us
                        //Who knows, just to keep in mind who cares etc
                        theScene.REG.get<jl::Camera>(theScene.myPlayerIndex).updateWithYawPitch(0.0f, 0.0f);

                        std::cout << "My starting position: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
                        theScene.worldReceived.store(true);
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
                                    std::vector<char> rfb(m.regFileSize);
                                    std::cout << "About to read that shit " << m.regFileSize * sizeof(char) << " bytes" << std::endl;
                                    boost::asio::read(*socket, boost::asio::buffer(rfb.data(), m.regFileSize * sizeof(char)), ec);
                                    if (!ec)
                                    {
                                        std::cout << "Got reg file \n";
                                        std::ofstream f2("snapshot.bin", std::ios::binary | std::ios::trunc);

                                        if(f2.is_open())
                                        {
                                            f2.write(rfb.data(), m.regFileSize * sizeof(char));
                                            f2.close();

                                            theScene.world->load("mpworld.txt", theScene.existingInvs, theScene.REG);

                                            auto view = theScene.REG.view<UUIDComponent>();
                                            for(auto entity : view)
                                            {
                                                auto uuid = view.get<UUIDComponent>(entity);
                                                if(uuid.uuid == theScene.settings.clientUID)
                                                {
                                                    theScene.myPlayerIndex = entity;
                                                }
                                            }
                                            std::cout << "Playerindex here: " << (int)theScene.myPlayerIndex << " \n";

                                            // if (theScene.existingInvs.contains(theScene.settings.clientUID))
                                            // {
                                            //     if (auto inv = loadInvFromFile("mpworld.txt", theScene.settings.clientUID))
                                            //     {
                                            //         theScene.getOur<InventoryComponent>().inventory = inv.value();
                                            //     }
                                            // }
                                        } else
                                        {
                                            std::cout << "Failed to load reg file \n" << std::endl;
                                        }
                                    } else
                                    {
                                        std::cout << "ec on reading reg file: " <<  ec.message() << std::endl;
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
