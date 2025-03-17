//
// Created by jack on 3/16/2025.
//

#ifndef NETWORKCOMPONENT_H
#define NETWORKCOMPONENT_H

#include "../PrecompHeader.h"

//We are not serializing this components fields but need it to exist!

struct NetworkComponent {
    std::weak_ptr<boost::asio::ip::tcp::socket> socket;
    bool receivedWorld = false;
    template<class Archive>
    void serialize(Archive& archive)
    {
        //deliberately empty
    }
};


#endif //NETWORKCOMPONENT_H
