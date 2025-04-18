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


inline void sendTextInChunks(tcp::socket* socket, const std::string& text)
{
    auto numChunks = (size_t)glm::ceil(text.size()/32);
    DGMessage m = TextChunkHeader{
        .numChunks = numChunks
    };

    boost::asio::write(*socket, boost::asio::buffer(&m, sizeof(DGMessage)));

    auto const chunks = std::string_view(text) | std::views::chunk(32);

    // for(auto chunk : chunks)
    // {
    //     DGMessage c = TextChunk{
    //         .sequenceNumber = (size_t)i,
    //         .data =
    //     };
    // }



}

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

    }
}

void read_from_server(tcp::socket* socket, std::atomic<bool>* shouldRun);

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
