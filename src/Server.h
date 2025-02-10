//
// Created by jack on 2/9/2025.
//

#ifndef SERVER_H
#define SERVER_H

#include "Camera.h"
#include "PrecompHeader.h"
#include "Network.h"
#include "world/DataMap.h"
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
        waitForMessage();
    }
private:

    void waitForMessage() {
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
        


    boost::asio::async_read(*m_socket, boost::asio::buffer(&m_message, sizeof(DGMessage)),
        [this, self](const boost::system::error_code& ec, std::size_t /*length*/) {
            if (!ec) {

                ///READ THE MESSAGA VARAIANTS EJRE

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
