//
// Created by jack on 2/9/2025.
//

#include "Client.h"
boost::lockfree::spsc_queue<DGMessage, boost::lockfree::capacity<512>> networkToMainBlockChangeQueue;

boost::lockfree::spsc_queue<DGMessage, boost::lockfree::capacity<512>> mainToNetworkBlockChangeQueue;


std::mutex networkMutex = {};
std::condition_variable networkCV = {};