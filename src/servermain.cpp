//
// Created by jack on 2/9/2025.
//


#include "Server.h"

int main()
{
    loadDM("serverworld.txt", serverWorld.userDataMap, serverReg, serverWorld.blockAreas, serverWorld.placedVoxModels);
    serverThreadFun(6969);
}
