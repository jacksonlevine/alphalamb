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
    virtual void erase(const IntTup& spot, bool locked = true) = 0;
    virtual void setLocked(const IntTup& spot, uint32_t block) = 0;
    virtual std::shared_mutex& mutex() = 0;

    virtual std::unique_lock<std::shared_mutex> getUniqueLock() = 0;

    class Iterator {
    public:
        virtual ~Iterator() = default;
        virtual bool hasNext() const = 0;
        virtual std::pair<const IntTup&, uint32_t&> next() = 0;
    };

    virtual std::unique_ptr<Iterator> createIterator() = 0;

};

#endif //USERDATAMAP_H
