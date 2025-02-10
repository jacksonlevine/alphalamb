//
// Created by jack on 2/9/2025.
//

#include "Client.h"
boost::lockfree::spsc_queue<BlockChange, boost::lockfree::capacity<64>> networkToMainBlockChangeQueue;

boost::lockfree::spsc_queue<BlockChange, boost::lockfree::capacity<64>> mainToNetworkBlockChangeQueue;