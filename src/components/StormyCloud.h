//
// Created by jack on 6/6/2025.
//

#ifndef STORMYCLOUD_H
#define STORMYCLOUD_H

#include "../PrecompHeader.h"

struct StormyCloud {
    std::chrono::time_point<std::chrono::steady_clock> start;
    StormyCloud() : start(std::chrono::steady_clock::now()) {}
};


#endif //STORMYCLOUD_H
