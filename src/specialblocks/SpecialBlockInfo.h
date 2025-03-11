//
// Created by jack on 3/10/2025.
//

#ifndef SPECIALBLOCKINFO_H
#define SPECIALBLOCKINFO_H
#include "../world/WorldRenderer.h"


class SpecialBlockInfo
{
public:
    virtual void meshMeDaddy(UsableMesh &mesh, uint32_t block, IntTup position, PxU32 &index, PxU32 &tindex) = 0;
};

#endif //SPECIALBLOCKINFO_H
