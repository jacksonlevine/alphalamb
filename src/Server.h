//
// Created by jack on 2/9/2025.
//

#ifndef SERVER_H
#define SERVER_H

#include "Camera.h"
#include "PrecompHeader.h"
#include "Network.h"
#include "world/DataMap.h"
#include "world/World.h"
using tcp = boost::asio::ip::tcp;

struct ServerPlayer
{
    std::weak_ptr<tcp::socket> socket;
    Controls controls;
    jl::Camera camera;
};

extern std::unordered_map<int, ServerPlayer> clients;

extern std::shared_mutex clientsMutex;

extern DataMap* serverUserDataMap;


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
        constexpr int seed = 101010;
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

        auto string = saveDM("serverworld.txt", serverUserDataMap);
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


        //Now start waiting for messages
        waitForMessage();
    }

    void waitForMessage() {
    auto self(shared_from_this());




    boost::asio::async_read(*m_socket, boost::asio::buffer(&m_message, sizeof(DGMessage)),
        [this, self](const boost::system::error_code& ec, std::size_t /*length*/) {
            if (!ec) {

                bool redistrib = false;

                visit([&](const auto& m) {
                    using T = std::decay_t<decltype(m)>;
                    if constexpr (std::is_same_v<T, WorldInfo>) {
                        std::cout << "Got world info " << m.seed << " \n";
                    }
                    else if constexpr (std::is_same_v<T, ControlsUpdate>) {
                        std::cout << "Got controls update \n";
                        redistrib = true;
                    }
                    else if constexpr (std::is_same_v<T, BlockSet>) {
                        std::cout << "Got block set \n";
                        serverUserDataMap->set(m.spot, m.block);
                        redistrib = true;
                    }
                    else if constexpr (std::is_same_v<T, FileTransferInit>) {
                        std::cout << "Got file transfer init \n";

                    }
                }, m_message);


                if (redistrib)
                {
                    auto lock = std::unique_lock<std::shared_mutex>(clientsMutex);
                    for(auto & [id, client] : clients)
                    {
                        if(true)//(client.myUuid != m_header.myUuid)
                        {
                            if(!client.socket.expired())
                            {
                                try
                                {
                                    boost::asio::async_write(*(client.socket.lock()), boost::asio::buffer(&m_message, sizeof(DGMessage)), [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                    {

                                    });

                                } catch (std::exception& e)
                                {
                                    std::cout << "Couldn't send: " <<  e.what() << "\n";
                                }

                            }

                        }
                    }
                }

                //Wait for next message
            waitForMessage();
            } else {
                std::cout << "Error reading message: " << ec.message() << std::endl;

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
                            .camera = jl::Camera{}
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
            // since we want multiple clients to connnect, wait for the next one by calling do_accept()
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

inline void launchServerThreadFun()
{
    std::thread serverThread(serverThreadFun, 25000);
    serverThread.detach();
}

#endif //SERVER_H
