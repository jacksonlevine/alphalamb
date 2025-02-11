//
// Created by jack on 2/9/2025.
//


#include "Server.h"

int main()
{
    loadDM("serverworld.txt", serverUserDataMap);
    serverThreadFun(6969);
}
