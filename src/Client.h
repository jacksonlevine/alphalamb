//
// Created by jack on 2/9/2025.
//

#ifndef CLIENT_H
#define CLIENT_H

#include "Network.h"
#include "PrecompHeader.h"

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

                visit([](const auto& m) {
                    using T = std::decay_t<decltype(m)>;
                    if constexpr (std::is_same_v<T, WorldInfo>) {
                        std::cout << "Got world info " << m.seed << " \n";
                    }
                    else if constexpr (std::is_same_v<T, ControlsUpdate>) {
                        std::cout << "Got controls update \n";
                    }
                    else if constexpr (std::is_same_v<T, FileTransferInit>) {
                        std::cout << "Got file transfer init \n";
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
