//
// Created by jack on 2/13/2025.
//

#ifndef LOCALSERVERIOCONTEXT_H
#define LOCALSERVERIOCONTEXT_H

#include "PrecompHeader.h"

extern std::unique_ptr<boost::asio::io_context> localserver_io_context;
extern std::atomic<bool> localserver_running;
extern std::thread localserver_thread;

extern boost::asio::thread_pool localserver_threadpool;

#endif //LOCALSERVERIOCONTEXT_H
