//
// Created by jack on 2/13/2025.
//
#include "LocalServerIOContext.h"
std::unique_ptr<boost::asio::io_context> localserver_io_context = nullptr;

std::atomic<bool> localserver_running = {false};
std::thread localserver_thread;