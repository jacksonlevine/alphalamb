//
// Created by jack on 1/27/2025.
//

#ifndef WORLDGENMETHOD_H
#define WORLDGENMETHOD_H
#include "../IntTup.h"
#include "../PrecompHeader.h"

///The interface the world gen method must satisfy
class WorldGenMethod {
public:
    virtual uint32_t get(IntTup spot) = 0;
};
#endif //WORLDGENMETHOD_H
