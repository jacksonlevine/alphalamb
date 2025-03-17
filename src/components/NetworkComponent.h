//
// Created by jack on 3/16/2025.
//

#ifndef NETWORKCOMPONENT_H
#define NETWORKCOMPONENT_H

#include "../PrecompHeader.h"

struct NetworkComponent {
    std::weak_ptr<boost::asio::ip::tcp::socket> socket;
    bool receivedWorld = false;
};


#endif //NETWORKCOMPONENT_H
