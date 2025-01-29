//
// Created by jack on 1/27/2025.
//

#ifndef USERDATAMAP_H
#define USERDATAMAP_H
#include "MaterialName.h"
#include "../IntTup.h"
#include "../PrecompHeader.h"

///The interface the user block data storage mechanism must satisfy
class UserDataMap {
public:
  virtual std::optional<uint32_t> get(IntTup spot) const = 0;
  virtual void set(IntTup spot, uint32_t block) = 0;
};

#endif //USERDATAMAP_H
