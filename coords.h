#pragma once

#include <boost/container_hash/hash.hpp>
#include <cstdint>

namespace otbm {

struct Coords {
  constexpr Coords() = default;
  constexpr Coords(uint16_t x, uint16_t y, uint8_t z) : x{x}, y{x}, z{z} {}
  constexpr bool operator==(const Coords &rhs) const { return x == rhs.x and y == rhs.y and z == rhs.z; }

  uint16_t x = 0;
  uint16_t y = 0;
  uint8_t z = 0;
};

} // namespace otbm

template <> struct std::hash<otbm::Coords> {
  size_t operator()(const otbm::Coords &coords) const {
    size_t seed = 0;
    boost::hash_combine(seed, coords.x);
    boost::hash_combine(seed, coords.y);
    boost::hash_combine(seed, coords.z);
    return seed;
  }
};