//
// Created by jack on 7/2/2025.
//

#ifndef MARCHINGCUBESLOOKUPS_H
#define MARCHINGCUBESLOOKUPS_H
#include "PrecompHeader.h"
#include "PhysXStuff.h"
inline static std::vector<PxVec3> marchingCubeVertices[256] = {
    {}, // config 0
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 1
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 2
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 3
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 4
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 5
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 6
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 7
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 8
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 9
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 10
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 11
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 12
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 13
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 14
    {PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 15
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 16
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f)}, // config 17
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 18
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 19
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 20
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 21
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 22
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 23
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 24
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 25
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 26
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 27
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 28
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 29
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 30
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 31
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 32
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 33
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f)}, // config 34
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 35
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 36
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 37
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 38
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 39
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 40
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 41
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 42
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 43
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 44
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 45
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 46
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 47
    {PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 48
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 49
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 50
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f)}, // config 51
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 52
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 53
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 54
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 55
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 56
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 57
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 58
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 59
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 60
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 61
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 62
    {PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 63
    {PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 64
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 65
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 66
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 67
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f)}, // config 68
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 69
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 70
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 71
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 72
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 73
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 74
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 75
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 76
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 77
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 78
    {PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 79
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 80
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 81
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 82
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 83
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 84
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f)}, // config 85
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 86
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 87
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 88
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 89
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 90
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 91
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 92
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 93
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 94
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 95
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 96
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 97
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 98
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 99
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 100
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 101
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f)}, // config 102
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 103
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 104
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 105
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 106
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 107
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 108
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 109
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 110
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 111
    {PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 112
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 113
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 114
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 115
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 116
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 117
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 118
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f)}, // config 119
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 120
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 121
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 122
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 123
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 124
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 125
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 126
    {PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 127
    {PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 128
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 129
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 130
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 131
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 132
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 133
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 134
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 135
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f)}, // config 136
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 137
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 138
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 139
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 140
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 141
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 142
    {PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 143
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 144
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 145
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 146
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 147
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 148
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 149
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 150
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 151
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 152
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f)}, // config 153
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 154
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 155
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 156
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 157
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 158
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 159
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 160
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 161
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 162
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 163
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 164
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 165
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 166
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 167
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 168
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 169
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f)}, // config 170
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 171
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 172
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 173
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 174
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 175
    {PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 176
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 177
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 178
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 179
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 180
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 181
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 182
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 183
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 184
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 185
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 186
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f)}, // config 187
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 188
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 189
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 190
    {PxVec3(0.5f, 1.0f, 1.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 191
    {PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 192
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 193
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 194
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 195
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 196
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 197
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 198
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 199
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 200
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 201
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 202
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 203
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f)}, // config 204
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 205
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 206
    {PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 207
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 208
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 209
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 210
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 211
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 212
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 213
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 214
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 215
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 216
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 217
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 218
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 219
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 220
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f)}, // config 221
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 222
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(1.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 223
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 224
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 225
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 226
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 227
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 228
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 229
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 230
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 231
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 232
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 233
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 234
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 235
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 236
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 237
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f)}, // config 238
    {PxVec3(0.5f, 1.0f, 0.0f), PxVec3(0.0f, 1.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 239
    {PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 240
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 241
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 242
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 0.5f, 1.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 243
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 244
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 245
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 246
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(1.0f, 0.5f, 1.0f)}, // config 247
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 248
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 249
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 250
    {PxVec3(0.5f, 0.0f, 1.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.5f, 1.0f)}, // config 251
    {PxVec3(1.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 252
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(1.0f, 0.0f, 0.5f), PxVec3(1.0f, 0.5f, 0.0f)}, // config 253
    {PxVec3(0.5f, 0.0f, 0.0f), PxVec3(0.0f, 0.0f, 0.5f), PxVec3(0.0f, 0.5f, 0.0f)}, // config 254
    {}, // config 255
};

// Marching Cubes Indices Lookup Table
inline static std::vector<PxU32> marchingCubeIndices[256] = {
    {}, // config 0
    {2, 1, 0}, // config 1
    {1, 2, 0}, // config 2
    {0, 2, 1, 3, 2, 0}, // config 3
    {1, 2, 0}, // config 4
    {3, 0, 2, 1, 0, 3}, // config 5
    {5, 2, 3, 4, 0, 1}, // config 6
    {1, 0, 4, 0, 3, 4, 3, 2, 4}, // config 7
    {1, 2, 0}, // config 8
    {4, 3, 0, 5, 1, 2}, // config 9
    {2, 1, 3, 0, 1, 2}, // config 10
    {1, 0, 2, 0, 4, 2, 4, 3, 2}, // config 11
    {2, 1, 3, 0, 1, 2}, // config 12
    {1, 0, 3, 0, 2, 3, 2, 4, 3}, // config 13
    {0, 1, 2, 1, 4, 2, 4, 3, 2}, // config 14
    {3, 1, 0, 2, 1, 3}, // config 15
    {1, 2, 0}, // config 16
    {1, 2, 3, 0, 2, 1}, // config 17
    {3, 4, 2, 1, 5, 0}, // config 18
    {4, 2, 0, 2, 3, 0, 3, 1, 0}, // config 19
    {2, 3, 4, 0, 1, 5}, // config 20
    {3, 4, 2, 4, 1, 2, 1, 0, 2}, // config 21
    {1, 7, 0, 4, 5, 6, 2, 3, 8}, // config 22
    {5, 2, 3, 1, 2, 5, 4, 2, 1, 0, 4, 1}, // config 23
    {3, 4, 2, 5, 0, 1}, // config 24
    {3, 4, 5, 0, 4, 3, 1, 2, 6}, // config 25
    {5, 1, 6, 0, 1, 5, 4, 2, 3}, // config 26
    {1, 0, 5, 2, 1, 5, 2, 3, 1, 2, 5, 4}, // config 27
    {1, 5, 0, 6, 5, 1, 3, 4, 2}, // config 28
    {1, 5, 4, 2, 3, 5, 2, 5, 1, 0, 2, 1}, // config 29
    {4, 2, 3, 0, 1, 5, 1, 7, 5, 7, 6, 5}, // config 30
    {4, 0, 1, 4, 2, 0, 4, 3, 2}, // config 31
    {1, 0, 2}, // config 32
    {3, 2, 5, 1, 0, 4}, // config 33
    {0, 3, 2, 1, 3, 0}, // config 34
    {2, 4, 3, 4, 1, 3, 1, 0, 3}, // config 35
    {3, 2, 4, 0, 1, 5}, // config 36
    {0, 6, 1, 4, 6, 0, 2, 5, 3}, // config 37
    {0, 5, 4, 1, 5, 0, 2, 3, 6}, // config 38
    {2, 0, 3, 5, 1, 0, 5, 0, 2, 4, 5, 2}, // config 39
    {1, 5, 0, 2, 4, 3}, // config 40
    {5, 4, 7, 6, 3, 0, 8, 1, 2}, // config 41
    {4, 3, 1, 3, 2, 1, 2, 0, 1}, // config 42
    {3, 0, 5, 2, 0, 3, 1, 0, 2, 4, 1, 2}, // config 43
    {5, 1, 6, 0, 1, 5, 4, 3, 2}, // config 44
    {5, 3, 2, 1, 0, 6, 0, 4, 6, 4, 7, 6}, // config 45
    {0, 1, 5, 3, 0, 5, 3, 2, 0, 3, 5, 4}, // config 46
    {2, 1, 0, 2, 3, 1, 2, 4, 3}, // config 47
    {3, 1, 2, 0, 1, 3}, // config 48
    {0, 4, 1, 4, 2, 1, 2, 3, 1}, // config 49
    {4, 0, 3, 0, 1, 3, 1, 2, 3}, // config 50
    {1, 2, 3, 0, 2, 1}, // config 51
    {3, 5, 2, 4, 5, 3, 1, 6, 0}, // config 52
    {3, 5, 1, 4, 3, 1, 4, 2, 3, 4, 1, 0}, // config 53
    {3, 7, 2, 6, 0, 5, 0, 1, 5, 1, 4, 5}, // config 54
    {0, 4, 1, 0, 3, 4, 0, 2, 3}, // config 55
    {5, 3, 4, 2, 3, 5, 6, 0, 1}, // config 56
    {1, 2, 7, 0, 6, 3, 6, 4, 3, 4, 5, 3}, // config 57
    {4, 2, 3, 1, 5, 2, 1, 2, 4, 0, 1, 4}, // config 58
    {2, 0, 4, 2, 1, 0, 2, 3, 1}, // config 59
    {2, 3, 4, 5, 2, 4, 6, 1, 7, 6, 0, 1}, // config 60
    {3, 6, 2, 2, 6, 5, 0, 4, 1}, // config 61
    {5, 2, 6, 6, 2, 3, 0, 1, 4}, // config 62
    {1, 3, 0, 0, 3, 2}, // config 63
    {2, 1, 0}, // config 64
    {2, 5, 3, 0, 4, 1}, // config 65
    {5, 3, 2, 1, 4, 0}, // config 66
    {4, 0, 5, 1, 0, 4, 6, 3, 2}, // config 67
    {3, 0, 1, 2, 0, 3}, // config 68
    {4, 3, 0, 3, 2, 0, 2, 1, 0}, // config 69
    {2, 5, 4, 3, 5, 2, 0, 1, 6}, // config 70
    {4, 3, 2, 0, 4, 2, 0, 5, 4, 0, 2, 1}, // config 71
    {3, 2, 5, 0, 1, 4}, // config 72
    {0, 6, 3, 5, 4, 8, 1, 2, 7}, // config 73
    {1, 4, 0, 5, 4, 1, 2, 6, 3}, // config 74
    {7, 3, 2, 1, 0, 4, 0, 6, 4, 6, 5, 4}, // config 75
    {2, 4, 3, 4, 0, 3, 0, 1, 3}, // config 76
    {3, 0, 4, 2, 0, 3, 1, 0, 2, 5, 1, 2}, // config 77
    {2, 1, 3, 4, 0, 1, 4, 1, 2, 5, 4, 2}, // config 78
    {4, 1, 0, 4, 2, 1, 4, 3, 2}, // config 79
    {2, 1, 3, 0, 1, 2}, // config 80
    {4, 1, 3, 1, 0, 3, 0, 2, 3}, // config 81
    {4, 3, 6, 2, 3, 4, 5, 0, 1}, // config 82
    {4, 1, 0, 3, 5, 1, 3, 1, 4, 2, 3, 4}, // config 83
    {1, 4, 0, 4, 2, 0, 2, 3, 0}, // config 84
    {3, 0, 2, 1, 0, 3}, // config 85
    {0, 1, 7, 3, 6, 2, 6, 4, 2, 4, 5, 2}, // config 86
    {2, 0, 4, 2, 1, 0, 2, 3, 1}, // config 87
    {3, 4, 2, 6, 4, 3, 1, 5, 0}, // config 88
    {2, 6, 1, 7, 3, 5, 3, 0, 5, 0, 4, 5}, // config 89
    {2, 3, 7, 4, 2, 7, 5, 1, 6, 5, 0, 1}, // config 90
    {4, 2, 5, 5, 2, 3, 1, 0, 6}, // config 91
    {1, 4, 2, 5, 1, 2, 5, 0, 1, 5, 2, 3}, // config 92
    {0, 4, 1, 0, 3, 4, 0, 2, 3}, // config 93
    {3, 6, 2, 2, 6, 5, 1, 4, 0}, // config 94
    {1, 3, 0, 0, 3, 2}, // config 95
    {5, 3, 2, 4, 1, 0}, // config 96
    {7, 3, 2, 4, 8, 5, 0, 6, 1}, // config 97
    {3, 0, 1, 2, 0, 3, 5, 4, 6}, // config 98
    {5, 4, 7, 2, 6, 3, 6, 1, 3, 1, 0, 3}, // config 99
    {5, 0, 1, 4, 0, 5, 3, 2, 6}, // config 100
    {2, 7, 3, 6, 5, 0, 5, 4, 0, 4, 1, 0}, // config 101
    {6, 2, 3, 7, 6, 3, 5, 0, 1, 5, 4, 0}, // config 102
    {1, 0, 4, 4, 0, 3, 6, 5, 2}, // config 103
    {0, 1, 7, 8, 5, 4, 6, 3, 2}, // config 104
    {8, 3, 0, 9, 5, 4, 7, 6, 11, 1, 2, 10}, // config 105
    {4, 7, 5, 6, 3, 1, 3, 2, 1, 2, 0, 1}, // config 106
    {5, 2, 6, 4, 7, 3, 0, 8, 1}, // config 107
    {3, 2, 6, 4, 7, 5, 7, 0, 5, 0, 1, 5}, // config 108
    {3, 4, 8, 2, 6, 5, 0, 7, 1}, // config 109
    {0, 1, 2, 2, 1, 5, 6, 3, 4}, // config 110
    {1, 2, 5, 3, 0, 4}, // config 111
    {0, 1, 3, 1, 4, 3, 4, 2, 3}, // config 112
    {1, 3, 5, 0, 3, 1, 2, 3, 0, 4, 2, 0}, // config 113
    {2, 3, 5, 0, 2, 5, 0, 1, 2, 0, 5, 4}, // config 114
    {1, 3, 4, 1, 2, 3, 1, 0, 2}, // config 115
    {2, 4, 5, 0, 1, 4, 0, 4, 2, 3, 0, 2}, // config 116
    {3, 4, 2, 3, 0, 4, 3, 1, 0}, // config 117
    {4, 5, 1, 1, 5, 2, 6, 0, 3}, // config 118
    {2, 3, 0, 0, 3, 1}, // config 119
    {6, 0, 1, 2, 3, 5, 3, 7, 5, 7, 4, 5}, // config 120
    {1, 0, 6, 2, 8, 3, 5, 7, 4}, // config 121
    {4, 0, 6, 6, 0, 1, 2, 3, 5}, // config 122
    {0, 5, 1, 3, 4, 2}, // config 123
    {1, 4, 0, 0, 4, 5, 3, 6, 2}, // config 124
    {5, 2, 3, 4, 1, 0}, // config 125
    {0, 1, 4, 3, 5, 2}, // config 126
    {2, 0, 1}, // config 127
    {1, 0, 2}, // config 128
    {4, 1, 0, 2, 5, 3}, // config 129
    {3, 2, 5, 0, 1, 4}, // config 130
    {0, 4, 1, 5, 4, 0, 2, 6, 3}, // config 131
    {1, 5, 0, 2, 4, 3}, // config 132
    {6, 0, 4, 1, 0, 6, 5, 3, 2}, // config 133
    {6, 0, 1, 3, 8, 2, 4, 7, 5}, // config 134
    {6, 3, 2, 1, 0, 7, 0, 5, 7, 5, 4, 7}, // config 135
    {0, 3, 2, 1, 3, 0}, // config 136
    {1, 5, 4, 2, 5, 1, 3, 0, 6}, // config 137
    {2, 4, 3, 4, 0, 3, 0, 1, 3}, // config 138
    {2, 0, 3, 4, 1, 0, 4, 0, 2, 5, 4, 2}, // config 139
    {4, 3, 1, 3, 2, 1, 2, 0, 1}, // config 140
    {1, 0, 4, 3, 1, 4, 3, 2, 1, 3, 4, 5}, // config 141
    {3, 1, 5, 2, 1, 3, 0, 1, 2, 4, 0, 2}, // config 142
    {3, 1, 0, 3, 4, 1, 3, 2, 4}, // config 143
    {5, 2, 1, 4, 0, 3}, // config 144
    {2, 1, 0, 5, 1, 2, 4, 3, 6}, // config 145
    {8, 4, 3, 5, 6, 2, 1, 7, 0}, // config 146
    {3, 7, 4, 6, 2, 0, 2, 5, 0, 5, 1, 0}, // config 147
    {6, 2, 5, 3, 7, 4, 1, 8, 0}, // config 148
    {4, 3, 6, 5, 7, 2, 7, 1, 2, 1, 0, 2}, // config 149
    {7, 8, 4, 5, 10, 6, 11, 2, 3, 9, 0, 1}, // config 150
    {7, 1, 0, 4, 5, 8, 2, 3, 6}, // config 151
    {4, 0, 1, 3, 0, 4, 2, 5, 6}, // config 152
    {7, 3, 0, 4, 7, 0, 6, 1, 2, 6, 5, 1}, // config 153
    {2, 5, 6, 3, 7, 4, 7, 0, 4, 0, 1, 4}, // config 154
    {1, 0, 5, 5, 0, 4, 6, 2, 3}, // config 155
    {5, 6, 2, 7, 4, 1, 4, 3, 1, 3, 0, 1}, // config 156
    {1, 0, 3, 3, 0, 2, 6, 4, 5}, // config 157
    {2, 3, 7, 5, 8, 4, 1, 6, 0}, // config 158
    {2, 3, 5, 0, 1, 4}, // config 159
    {0, 3, 1, 2, 3, 0}, // config 160
    {2, 6, 3, 5, 6, 2, 0, 4, 1}, // config 161
    {1, 4, 0, 4, 3, 0, 3, 2, 0}, // config 162
    {0, 5, 3, 4, 0, 3, 4, 1, 0, 4, 3, 2}, // config 163
    {5, 2, 4, 3, 2, 5, 6, 0, 1}, // config 164
    {4, 7, 1, 0, 4, 1, 2, 6, 3, 2, 5, 6}, // config 165
    {2, 3, 7, 1, 6, 0, 6, 5, 0, 5, 4, 0}, // config 166
    {2, 4, 3, 3, 4, 6, 0, 5, 1}, // config 167
    {4, 0, 2, 0, 1, 2, 1, 3, 2}, // config 168
    {0, 6, 3, 7, 1, 4, 1, 2, 4, 2, 5, 4}, // config 169
    {2, 1, 3, 0, 1, 2}, // config 170
    {0, 4, 1, 0, 2, 4, 0, 3, 2}, // config 171
    {4, 3, 2, 1, 5, 3, 1, 3, 4, 0, 1, 4}, // config 172
    {6, 3, 4, 4, 3, 2, 1, 0, 5}, // config 173
    {3, 1, 4, 3, 0, 1, 3, 2, 0}, // config 174
    {3, 1, 2, 2, 1, 0}, // config 175
    {0, 1, 4, 1, 2, 4, 2, 3, 4}, // config 176
    {0, 3, 1, 5, 2, 3, 5, 3, 0, 4, 5, 0}, // config 177
    {3, 5, 2, 4, 5, 3, 1, 5, 4, 0, 1, 4}, // config 178
    {3, 4, 2, 3, 0, 4, 3, 1, 0}, // config 179
    {7, 0, 1, 2, 3, 6, 3, 4, 6, 4, 5, 6}, // config 180
    {0, 4, 1, 1, 4, 5, 3, 6, 2}, // config 181
    {3, 6, 0, 8, 4, 5, 7, 2, 1}, // config 182
    {5, 2, 3, 4, 1, 0}, // config 183
    {5, 0, 1, 3, 5, 1, 3, 4, 5, 3, 1, 2}, // config 184
    {4, 5, 2, 2, 5, 3, 6, 1, 0}, // config 185
    {0, 3, 4, 0, 2, 3, 0, 1, 2}, // config 186
    {1, 0, 3, 3, 0, 2}, // config 187
    {5, 0, 4, 4, 0, 1, 2, 3, 6}, // config 188
    {2, 3, 5, 0, 4, 1}, // config 189
    {3, 5, 2, 1, 4, 0}, // config 190
    {0, 1, 2}, // config 191
    {0, 3, 1, 2, 3, 0}, // config 192
    {6, 2, 5, 3, 2, 6, 4, 1, 0}, // config 193
    {2, 6, 3, 5, 6, 2, 1, 4, 0}, // config 194
    {6, 7, 3, 2, 6, 3, 0, 4, 1, 0, 5, 4}, // config 195
    {4, 0, 2, 0, 1, 2, 1, 3, 2}, // config 196
    {5, 3, 2, 0, 4, 3, 0, 3, 5, 1, 0, 5}, // config 197
    {1, 6, 0, 7, 2, 4, 2, 3, 4, 3, 5, 4}, // config 198
    {4, 3, 5, 5, 3, 2, 1, 0, 6}, // config 199
    {1, 4, 0, 4, 3, 0, 3, 2, 0}, // config 200
    {3, 0, 6, 2, 7, 1, 7, 5, 1, 5, 4, 1}, // config 201
    {1, 5, 3, 4, 1, 3, 4, 0, 1, 4, 3, 2}, // config 202
    {2, 5, 3, 3, 5, 4, 0, 6, 1}, // config 203
    {3, 0, 1, 2, 0, 3}, // config 204
    {3, 0, 4, 3, 1, 0, 3, 2, 1}, // config 205
    {1, 4, 0, 1, 2, 4, 1, 3, 2}, // config 206
    {2, 1, 3, 3, 1, 0}, // config 207
    {0, 1, 2, 1, 3, 2, 3, 4, 2}, // config 208
    {5, 1, 0, 3, 5, 0, 3, 4, 5, 3, 0, 2}, // config 209
    {5, 0, 1, 2, 3, 4, 3, 6, 4, 6, 7, 4}, // config 210
    {6, 1, 5, 5, 1, 0, 2, 3, 4}, // config 211
    {4, 0, 1, 2, 0, 4, 5, 0, 2, 3, 5, 2}, // config 212
    {1, 3, 4, 1, 2, 3, 1, 0, 2}, // config 213
    {7, 4, 5, 0, 3, 6, 2, 1, 8}, // config 214
    {0, 5, 1, 3, 4, 2}, // config 215
    {2, 5, 4, 0, 1, 5, 0, 5, 2, 3, 0, 2}, // config 216
    {4, 5, 0, 0, 5, 1, 6, 3, 2}, // config 217
    {1, 6, 0, 0, 6, 4, 3, 5, 2}, // config 218
    {4, 2, 3, 5, 1, 0}, // config 219
    {3, 4, 2, 3, 1, 4, 3, 0, 1}, // config 220
    {2, 3, 0, 0, 3, 1}, // config 221
    {5, 2, 3, 4, 0, 1}, // config 222
    {2, 0, 1}, // config 223
    {1, 0, 4, 0, 2, 4, 2, 3, 4}, // config 224
    {4, 1, 0, 3, 2, 7, 2, 5, 7, 5, 6, 7}, // config 225
    {3, 4, 5, 0, 1, 4, 0, 4, 3, 2, 0, 3}, // config 226
    {0, 5, 1, 1, 5, 6, 2, 4, 3}, // config 227
    {5, 0, 1, 2, 5, 1, 2, 4, 5, 2, 1, 3}, // config 228
    {6, 1, 5, 5, 1, 0, 3, 2, 4}, // config 229
    {5, 4, 3, 3, 4, 0, 6, 2, 1}, // config 230
    {2, 4, 3, 1, 0, 5}, // config 231
    {2, 5, 3, 4, 5, 2, 1, 5, 4, 0, 1, 4}, // config 232
    {0, 7, 1, 6, 5, 4, 8, 3, 2}, // config 233
    {2, 4, 3, 2, 1, 4, 2, 0, 1}, // config 234
    {4, 3, 2, 5, 1, 0}, // config 235
    {0, 2, 4, 0, 3, 2, 0, 1, 3}, // config 236
    {2, 4, 3, 0, 5, 1}, // config 237
    {3, 2, 1, 1, 2, 0}, // config 238
    {0, 2, 1}, // config 239
    {0, 2, 3, 1, 2, 0}, // config 240
    {2, 1, 0, 2, 4, 1, 2, 3, 4}, // config 241
    {3, 0, 1, 3, 2, 0, 3, 4, 2}, // config 242
    {3, 1, 2, 2, 1, 0}, // config 243
    {2, 0, 1, 2, 4, 0, 2, 3, 4}, // config 244
    {3, 1, 2, 2, 1, 0}, // config 245
    {0, 3, 4, 2, 1, 5}, // config 246
    {0, 2, 1}, // config 247
    {4, 0, 1, 4, 3, 0, 4, 2, 3}, // config 248
    {3, 2, 5, 1, 0, 4}, // config 249
    {2, 0, 3, 3, 0, 1}, // config 250
    {0, 2, 1}, // config 251
    {1, 2, 0, 0, 2, 3}, // config 252
    {0, 2, 1}, // config 253
    {0, 1, 2}, // config 254
    {}, // config 255
};
#endif //MARCHINGCUBESLOOKUPS_H
