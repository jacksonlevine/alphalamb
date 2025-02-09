//
// Created by jack on 1/27/2025.
//

#ifndef USERDATAMAP_H
#define USERDATAMAP_H
#include "MaterialName.h"
#include "../IntTup.h"
#include "../PrecompHeader.h"

///The interface the user block data storage mechanism must satisfy
class DataMap
{
public:
    virtual std::optional<uint32_t> get(const IntTup& spot) const = 0;
    virtual std::optional<uint32_t> getLocked(const IntTup& spot) const = 0;
    virtual void clear() = 0;
    virtual void set(const IntTup& spot, uint32_t block) = 0;
    virtual std::shared_mutex& mutex() = 0;
};

#endif //USERDATAMAP_H
