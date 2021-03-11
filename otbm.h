#pragma once

#include "coords.h"
#include "otb.h"
#include "otbi.h"

#include <cstdint>
#include <tsl/robin_map.h>
#include <utility>

namespace otbm {

enum {
  TILESTATE_NONE = 0,

  TILESTATE_FLOORCHANGE_DOWN = 1 << 0,
  TILESTATE_FLOORCHANGE_NORTH = 1 << 1,
  TILESTATE_FLOORCHANGE_SOUTH = 1 << 2,
  TILESTATE_FLOORCHANGE_EAST = 1 << 3,
  TILESTATE_FLOORCHANGE_WEST = 1 << 4,
  TILESTATE_FLOORCHANGE_SOUTH_ALT = 1 << 5,
  TILESTATE_FLOORCHANGE_EAST_ALT = 1 << 6,
  TILESTATE_PROTECTIONZONE = 1 << 7,
  TILESTATE_NOPVPZONE = 1 << 8,
  TILESTATE_NOLOGOUT = 1 << 9,
  TILESTATE_PVPZONE = 1 << 10,
  TILESTATE_TELEPORT = 1 << 11,
  TILESTATE_MAGICFIELD = 1 << 12,
  TILESTATE_MAILBOX = 1 << 13,
  TILESTATE_TRASHHOLDER = 1 << 14,
  TILESTATE_BED = 1 << 15,
  TILESTATE_DEPOT = 1 << 16,
  TILESTATE_BLOCKSOLID = 1 << 17,
  TILESTATE_BLOCKPATH = 1 << 18,
  TILESTATE_IMMOVABLEBLOCKSOLID = 1 << 19,
  TILESTATE_IMMOVABLEBLOCKPATH = 1 << 20,
  TILESTATE_IMMOVABLENOFIELDBLOCKPATH = 1 << 21,
  TILESTATE_NOFIELDBLOCKPATH = 1 << 22,
  TILESTATE_SUPPORTS_HANGABLE = 1 << 23,

  TILESTATE_FLOORCHANGE = TILESTATE_FLOORCHANGE_DOWN | TILESTATE_FLOORCHANGE_NORTH | TILESTATE_FLOORCHANGE_SOUTH | TILESTATE_FLOORCHANGE_EAST |
                          TILESTATE_FLOORCHANGE_WEST | TILESTATE_FLOORCHANGE_SOUTH_ALT | TILESTATE_FLOORCHANGE_EAST_ALT,
};

#pragma pack(1)
struct Header {
  uint32_t version;
  uint16_t width;
  uint16_t height;
  uint32_t majorVersionItems;
  uint32_t minorVersionItems;
};
#pragma pack()

class Tile {
public:
  explicit Tile(otb::Item ground, uint32_t flags) : ground{std::move(ground)}, flags{flags} {}
  auto emplace_item(otb::Item &&item) { return items.emplace_back(std::forward<otb::Item>(item)); }

private:
  std::vector<otb::Item> items = {};
  otb::Item ground;
  uint32_t flags;
};

struct Town {
  Town(uint32_t id, std::string &&name, const Coords &temple) : id{id}, name{std::move(name)}, temple{temple} {}

  uint32_t id;
  std::string name;
  Coords temple;
};

using Tiles = tsl::robin_map<Coords, Tile>;
using Towns = tsl::robin_map<uint16_t, Town>;
using Waypoints = tsl::robin_map<std::string, Coords>;

class Map {
public:
  Map(const Header &header, Tiles &&tiles, Towns &&towns, Waypoints &&waypoints)
      : header{header}, tiles{std::move(tiles)}, towns{std::move(towns)}, waypoints{std::move(waypoints)} {}

private:
  Header header;
  Tiles tiles;
  Towns towns;
  Waypoints waypoints;
};

Map load(const std::string &filename, const otbi::Items &items);

} // namespace otbm
