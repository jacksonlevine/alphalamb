//
// Created by jack on 2/26/2025.
//

#ifndef CLIENTUID_H
#define CLIENTUID_H

#include "PrecompHeader.h"


using ClientUID = boost::uuids::uuid;


inline std::istream& operator>>(std::istream& is, ClientUID& uid)
{
    std::string uuidStr;
    is >> uuidStr;

    try {
        uid = boost::uuids::string_generator()(uuidStr);
    } catch (...) {
        is.setstate(std::ios::failbit);
    }

    return is;
}

inline std::ostream& operator<<(std::ostream& os, const ClientUID& uid)
{
    os << boost::lexical_cast<std::string>(uid);
    return os;
}


#endif //CLIENTUID_H
