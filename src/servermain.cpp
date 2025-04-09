//
// Created by jack on 2/9/2025.
//


#include "Server.h"

int main()
{
    loadDM("serverworld.txt", &serverWorld, serverReg, serverWorld.blockAreas, serverWorld.placedVoxModels);
    serverThreadFun(6969);
}
