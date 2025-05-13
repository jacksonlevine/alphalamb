//
// Created by jack on 5/12/2025.
//

#ifndef JL484_VEC3_H
#define JL484_VEC3_H

#include "PrecompHeader.h"

//used for chunk local positions
struct jl484_vec3 {
  uint8_t x : 4 {0};
  uint8_t z : 4 {0};
  uint8_t y : 8 {0};
  jl484_vec3() = default;
  jl484_vec3(uint8_t x, uint8_t y, uint8_t z) : x(x), y(y), z(z)
  {
#ifndef NDEBUG
  assert(x < 16 && "jl484_vec3::x is out of range");
  assert(y < 256 && "jl484_vec3::y is out of range");
  assert(z < 16 && "jl484_vec3::z is out of range");
#endif
  };
  bool operator==(const jl484_vec3& other) const
  {
    return x == other.x && y == other.y && z == other.z;
  };
};


struct jl484_vec3_hash
{
  std::size_t operator()(const jl484_vec3& x) const
  {
    return (static_cast<std::size_t>(x.x) << 16) |
           (static_cast<std::size_t>(x.y) << 8) |
           (static_cast<std::size_t>(x.z));
  }
};
#endif //JL484_VEC3_H
