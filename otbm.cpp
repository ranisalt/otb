#include "otbm.h"
#include "stream.h"

#include <fmt/format.h>
#include <optional>
#include <stdexcept>

template <> struct fmt::formatter<otbm::Coords> {
  constexpr auto parse(format_parse_context &ctx) {
    // Parse the presentation format and store it in the formatter:
    auto it = ctx.begin(), end = ctx.end();

    // Check if reached the end of the range:
    if (it != end && *it != '}')
      throw format_error("invalid format");

    // Return an iterator past the end of the parsed range:
    return it;
  }

  template <typename FormatContext> auto format(const otbm::Coords &c, FormatContext &ctx) { return format_to(ctx.out(), "({:d}, {:d}, {:d})", c.x, c.y, c.z); }
};

namespace otbm {

namespace {

enum {
  ATTR_DESCRIPTION = 1,
  ATTR_EXT_FILE = 2,
  ATTR_TILE_FLAGS = 3,
  ATTR_ACTION_ID = 4,
  ATTR_UNIQUE_ID = 5,
  ATTR_TEXT = 6,
  ATTR_DESC = 7,
  ATTR_TELE_DEST = 8,
  ATTR_ITEM = 9,
  ATTR_DEPOT_ID = 10,
  ATTR_EXT_SPAWN_FILE = 11,
  ATTR_RUNE_CHARGES = 12,
  ATTR_EXT_HOUSE_FILE = 13,
  ATTR_HOUSEDOORID = 14,
  ATTR_COUNT = 15,
  ATTR_DURATION = 16,
  ATTR_DECAYING_STATE = 17,
  ATTR_WRITTENDATE = 18,
  ATTR_WRITTENBY = 19,
  ATTR_SLEEPERGUID = 20,
  ATTR_SLEEPSTART = 21,
  ATTR_CHARGES = 22,
  ATTR_CONTAINER_ITEMS = 23,
  ATTR_NAME = 24,
  ATTR_ARTICLE = 25,
  ATTR_PLURALNAME = 26,
  ATTR_WEIGHT = 27,
  ATTR_ATTACK = 28,
  ATTR_DEFENSE = 29,
  ATTR_EXTRADEFENSE = 30,
  ATTR_ARMOR = 31,
  ATTR_HITCHANCE = 32,
  ATTR_SHOOTRANGE = 33,
  ATTR_CUSTOM_ATTRIBUTES = 34,
  ATTR_DECAYTO = 35,
  ATTR_WRAPID = 36,
  ATTR_STOREITEM = 37
};

enum {
  ITEM_BROWSEFIELD = 460, // for internal use

  ITEM_FIREFIELD_PVP_FULL = 1487,
  ITEM_FIREFIELD_PVP_MEDIUM = 1488,
  ITEM_FIREFIELD_PVP_SMALL = 1489,
  ITEM_FIREFIELD_PERSISTENT_FULL = 1492,
  ITEM_FIREFIELD_PERSISTENT_MEDIUM = 1493,
  ITEM_FIREFIELD_PERSISTENT_SMALL = 1494,
  ITEM_FIREFIELD_NOPVP = 1500,

  ITEM_POISONFIELD_PVP = 1490,
  ITEM_POISONFIELD_PERSISTENT = 1496,
  ITEM_POISONFIELD_NOPVP = 1503,

  ITEM_ENERGYFIELD_PVP = 1491,
  ITEM_ENERGYFIELD_PERSISTENT = 1495,
  ITEM_ENERGYFIELD_NOPVP = 1504,

  ITEM_MAGICWALL = 1497,
  ITEM_MAGICWALL_PERSISTENT = 1498,
  ITEM_MAGICWALL_SAFE = 11098,
  ITEM_MAGICWALL_NOPVP = 20669,

  ITEM_WILDGROWTH = 1499,
  ITEM_WILDGROWTH_PERSISTENT = 2721,
  ITEM_WILDGROWTH_SAFE = 11099,
  ITEM_WILDGROWTH_NOPVP = 20670,

  ITEM_BAG = 1987,
  ITEM_SHOPPING_BAG = 23782,

  ITEM_GOLD_COIN = 2148,
  ITEM_PLATINUM_COIN = 2152,
  ITEM_CRYSTAL_COIN = 2160,
  ITEM_STORE_COIN = 24774, // in-game store currency

  ITEM_DEPOT = 2594,
  ITEM_LOCKER1 = 2589,
  ITEM_INBOX = 14404,
  ITEM_MARKET = 14405,
  ITEM_STORE_INBOX = 26052,
  ITEM_DEPOT_BOX_I = 25453,
  ITEM_DEPOT_BOX_II = 25454,
  ITEM_DEPOT_BOX_III = 25455,
  ITEM_DEPOT_BOX_IV = 25456,
  ITEM_DEPOT_BOX_V = 25457,
  ITEM_DEPOT_BOX_VI = 25458,
  ITEM_DEPOT_BOX_VII = 25459,
  ITEM_DEPOT_BOX_VIII = 25460,
  ITEM_DEPOT_BOX_IX = 25461,
  ITEM_DEPOT_BOX_X = 25462,
  ITEM_DEPOT_BOX_XI = 25463,
  ITEM_DEPOT_BOX_XII = 25464,
  ITEM_DEPOT_BOX_XIII = 25465,
  ITEM_DEPOT_BOX_XIV = 25466,
  ITEM_DEPOT_BOX_XV = 25467,
  ITEM_DEPOT_BOX_XVI = 25468,
  ITEM_DEPOT_BOX_XVII = 25469,

  ITEM_MALE_CORPSE = 3058,
  ITEM_FEMALE_CORPSE = 3065,

  ITEM_FULLSPLASH = 2016,
  ITEM_SMALLSPLASH = 2019,

  ITEM_PARCEL = 2595,
  ITEM_LETTER = 2597,
  ITEM_LETTER_STAMPED = 2598,
  ITEM_LABEL = 2599,

  ITEM_AMULETOFLOSS = 2173,

  ITEM_DOCUMENT_RO = 1968, // read-only
};

enum {
  NODETYPE_ROOTV1 = 1,
  NODETYPE_MAP_DATA = 2,
  NODETYPE_ITEM_DEF = 3,
  NODETYPE_TILE_AREA = 4,
  NODETYPE_TILE = 5,
  NODETYPE_ITEM = 6,
  NODETYPE_TILE_SQUARE = 7,
  NODETYPE_TILE_REF = 8,
  NODETYPE_SPAWNS = 9,
  NODETYPE_SPAWN_AREA = 10,
  NODETYPE_MONSTER = 11,
  NODETYPE_TOWNS = 12,
  NODETYPE_TOWN = 13,
  NODETYPE_HOUSETILE = 14,
  NODETYPE_WAYPOINTS = 15,
  NODETYPE_WAYPOINT = 16,
};

enum {
  TILEFLAG_PROTECTIONZONE = 1 << 0,
  TILEFLAG_NOPVPZONE = 1 << 2,
  TILEFLAG_NOLOGOUT = 1 << 3,
  TILEFLAG_PVPZONE = 1 << 4,
};

uint16_t get_persistent_id(uint16_t id) {
  switch (id) {
  case ITEM_FIREFIELD_PVP_FULL:
    return ITEM_FIREFIELD_PERSISTENT_FULL;

  case ITEM_FIREFIELD_PVP_MEDIUM:
    return ITEM_FIREFIELD_PERSISTENT_MEDIUM;

  case ITEM_FIREFIELD_PVP_SMALL:
    return ITEM_FIREFIELD_PERSISTENT_SMALL;

  case ITEM_ENERGYFIELD_PVP:
    return ITEM_ENERGYFIELD_PERSISTENT;

  case ITEM_POISONFIELD_PVP:
    return ITEM_POISONFIELD_PERSISTENT;

  case ITEM_MAGICWALL:
    return ITEM_MAGICWALL_PERSISTENT;

  case ITEM_WILDGROWTH:
    return ITEM_WILDGROWTH_PERSISTENT;

  default:
    return id;
  }
}

auto parse_map_attributes(otb::node node) {
  struct {
    std::string description, spawns, houses;
  } out;

  auto first = node.props_begin, last = node.props_end;
  while (first != last) {
    switch (auto attr = read<uint8_t>(first, last)) {
    case ATTR_DESCRIPTION: {
      auto len = read<uint16_t>(first, last);
      out.description = read_string(first, last, len);
      break;
    }

    case ATTR_EXT_SPAWN_FILE: {
      auto len = read<uint16_t>(first, last);
      out.spawns = read_string(first, last, len);
      break;
    }

    case ATTR_EXT_HOUSE_FILE: {
      auto len = read<uint16_t>(first, last);
      out.houses = read_string(first, last, len);
      break;
    }

    default:
      throw std::invalid_argument(fmt::format("Unknown map attribute {:d}\n", attr));
    }
  }

  return out;
}

struct House {
  std::vector<Coords> tiles;
};

template <class T> void parse_tile_area(otb::node node, const otbi::Items &items, T &&callback) {
  auto coords = read<Coords>(node.props_begin, node.props_end);

  tsl::robin_map<uint32_t, House> houses;
  std::optional<Tile> tile;
  std::optional<otb::Item> ground_item;
  for (auto tile_node : node.children) {
    tile.reset();
    ground_item.reset();

    if (tile_node.type != NODETYPE_TILE and tile_node.type != NODETYPE_HOUSETILE) {
      throw std::invalid_argument(fmt::format("Unknown tile node: {:d}", tile_node.type));
    }

    auto first = tile_node.props_begin, last = tile_node.props_end;
    uint16_t x = coords.x + read<uint8_t>(first, last);
    uint16_t y = coords.y + read<uint8_t>(first, last);
    uint8_t z = coords.z;

    uint32_t house_id = 0;
    if (tile_node.type == NODETYPE_HOUSETILE) {
      house_id = read<uint32_t>(first, last);
      houses[house_id].tiles.emplace_back(x, y, z);
    }

    int tile_flags = TILESTATE_NONE;
    while (first != last) {
      switch (auto attr = read<uint8_t>(first, last)) {
      case ATTR_TILE_FLAGS: {
        auto flags = read<uint32_t>(first, last);

        if (flags & TILEFLAG_PROTECTIONZONE) {
          tile_flags |= TILESTATE_PROTECTIONZONE;
        } else if (flags & TILEFLAG_NOPVPZONE) {
          tile_flags |= TILESTATE_NOPVPZONE;
        } else if (flags & TILEFLAG_PVPZONE) {
          tile_flags |= TILESTATE_PVPZONE;
        }

        if (flags & TILEFLAG_NOLOGOUT) {
          tile_flags |= TILESTATE_NOLOGOUT;
        }

        break;
      }

      case ATTR_ITEM: {
        auto id = get_persistent_id(read<uint16_t>(first, last));
        auto type = items.at(id);
        auto item = otb::Item{&type};

        if (house_id != 0 and type.moveable()) {
          fmt::print("[Warning] Moveable item with ID {:d} in house {:d} @ {}.\n", id, house_id, Coords{x, y, z});
          break;
        } else {
          if (tile) {
            tile->emplace_item(std::move(item));
          } else if (type.is_ground_tile()) {
            ground_item.emplace(std::move(item));
          } else {
            tile.emplace(*ground_item, tile_flags);
            ground_item.reset();
          }
        }

        break;
      }

      default:
        throw std::invalid_argument(fmt::format("Unknown tile attribute {:d} @ {}", attr, Coords{x, y, z}));
      }
    }

    for (auto item_node : tile_node.children) {
      if (item_node.type != NODETYPE_ITEM) {
        throw std::invalid_argument(fmt::format("Unknown node type: {:d}", item_node.type));
      }

      auto first = item_node.props_begin, last = item_node.props_end;
      auto id = get_persistent_id(read<uint16_t>(first, last));
      auto type = items.at(id);
      auto item = otb::Item{&type};

      while (first != last) {
        auto attr = read<uint8_t>(first, last);
        switch (attr) {
        case ATTR_CHARGES:
        case ATTR_COUNT:
        case ATTR_RUNE_CHARGES:
          item.subtype(read<uint8_t>(first, last));
          break;

        case ATTR_ACTION_ID:
          item.action_id = read<uint16_t>(first, last);
          break;

        case ATTR_UNIQUE_ID:
          item.unique_id = read<uint16_t>(first, last);
          break;

        case ATTR_TEXT: {
          auto len = read<uint16_t>(first, last);
          item.text = read_string(first, last, len);
          break;
        }

        case ATTR_WRITTENDATE:
          item.written_at = read<uint32_t>(first, last);
          break;

        case ATTR_WRITTENBY: {
          auto len = read<uint16_t>(first, last);
          item.writer = read_string(first, last, len);
          break;
        }

        case ATTR_DESC: {
          auto len = read<uint16_t>(first, last);
          item.description = read_string(first, last, len);
          break;
        }

        case ATTR_DURATION:
          item.duration = std::max<int32_t>(0, read<int32_t>(first, last));
          break;

        case ATTR_DECAYING_STATE:
          // TODO
          read<uint8_t>(first, last);
          break;

        case ATTR_NAME: {
          auto len = read<uint16_t>(first, last);
          item.name = read_string(first, last, len);
          break;
        }

        case ATTR_ARTICLE: {
          auto len = read<uint16_t>(first, last);
          item.article = read_string(first, last, len);
          break;
        }

        case ATTR_PLURALNAME: {
          auto len = read<uint16_t>(first, last);
          item.plural_name = read_string(first, last, len);
          break;
        }

        case ATTR_WEIGHT:
          item.weight = read<uint32_t>(first, last);
          break;

        case ATTR_ATTACK:
          item.attack = read<int32_t>(first, last);
          break;

        case ATTR_DEFENSE:
          item.defense = read<int32_t>(first, last);
          break;

        case ATTR_EXTRADEFENSE:
          item.extra_defense = read<int32_t>(first, last);
          break;

        case ATTR_ARMOR:
          item.armor = read<int32_t>(first, last);
          break;

        case ATTR_HITCHANCE:
          item.hit_chance = read<uint8_t>(first, last);
          break;

        case ATTR_SHOOTRANGE:
          item.shoot_range = read<uint8_t>(first, last);
          break;

        case ATTR_DECAYTO:
          item.decay_to = read<int32_t>(first, last);
          break;

        case ATTR_WRAPID:
          item.wrap_id = read<uint16_t>(first, last);
          break;

        case ATTR_STOREITEM:
          item.store_item = read<uint8_t>(first, last);
          break;

          // these should be handled through derived classes
          // if these are called then something has changed in the items.xml since the map was saved
          // just read the values

        case ATTR_DEPOT_ID:
          skip(first, last, 2);
          break;

        case ATTR_HOUSEDOORID:
          skip(first, last, 1);
          break;

        case ATTR_SLEEPERGUID:
        case ATTR_SLEEPSTART:
          skip(first, last, 4);
          break;

        case ATTR_TELE_DEST:
          skip(first, last, 5);
          break;

        case ATTR_CONTAINER_ITEMS:
          throw std::invalid_argument("Invalid attribute: container items");

        case ATTR_CUSTOM_ATTRIBUTES: {
          uint64_t len = read<uint64_t>(first, last);

          for (uint64_t i = 0; i < len; ++i) {
            auto key_len = read<uint16_t>(first, last);
            auto key = read_string(first, last, key_len);

            auto val = otb::Item::attribute{};

            switch (read<uint8_t>(first, last)) {
            case 1: {
              auto val_len = read<uint16_t>(first, last);
              val = read_string(first, last, val_len);
              break;
            }

            case 2:
              val = read<int64_t>(first, last);
              break;

            case 3:
              val = read<double>(first, last);
              break;

            case 4:
              val = read<bool>(first, last);
              break;
            }

            item.custom_attributes.emplace(key, val);
          }
          break;
        }

        default:
          std::vector<int> bytes(item_node.props_begin, item_node.props_end);
          fmt::print("Unknown item attribute: {:d} (id: {:d}, name: {:s}, bytes: {})\n", attr, id, type.name(), fmt::join(bytes, " "));
        }
      }
    }

    callback(Coords{x, y, z}, std::move(*tile));
  }
}

template <class T> void parse_towns(otb::node node, T callback) {
  for (auto town_node : node.children) {
    if (town_node.type != NODETYPE_TOWN) {
      throw std::invalid_argument(fmt::format("Unknown town node: {:d}", town_node.type));
    }

    auto first = town_node.props_begin, last = town_node.props_end;
    auto town_id = read<uint32_t>(first, last);

    auto name_len = read<uint16_t>(first, last);
    auto name = read_string(first, last, name_len);

    auto temple = read<Coords>(first, last);
    callback(town_id, Town{town_id, name, temple});
    fmt::print(">>> Town {:d} ({:s} @ {})\n", town_id, name, temple);
  }
}

template <class T> void parse_waypoints(otb::node node, T callback) {
  for (auto waypoint_node : node.children) {
    if (waypoint_node.type != NODETYPE_WAYPOINT) {
      throw std::invalid_argument(fmt::format("Unknown waypoint node: {:d}", waypoint_node.type));
    }

    auto first = waypoint_node.props_begin, last = waypoint_node.props_end;

    auto name_len = read<uint16_t>(first, last);
    auto name = read_string(first, last, name_len);

    auto coords = read<Coords>(first, last);
    callback(std::move(name), std::move(coords));
    fmt::print(">>> Waypoint {:s}: {}.\n", name, coords);
  }
}

} // namespace

Map load(const std::string &filename, const otbi::Items &items) {
  auto loader = otb::load(filename, "OTBM");
  auto first = loader.begin();
  auto header = read<Header>(first, loader.end());

  if (header.version == 0) {
    throw std::invalid_argument("This map need to be upgraded by using the latest map editor version "
                                "to be able to load correctly.");
  } else if (header.version > 2) {
    throw std::invalid_argument("Unknown OTBM version detected.");
  }

  fmt::print("> Map size: {:d}x{:d}.\n", header.width, header.height);

  if (loader.children().size() != 1 or loader.children().front().type != NODETYPE_MAP_DATA) {
    throw std::invalid_argument("Could not read data node.");
  }

  auto map_node = loader.children().front();
  auto attributes = parse_map_attributes(map_node);
  fmt::print(">> Description: '{:s}'\n>> Houses: '{:s}'\n>> Spawns: '{:s}'\n", attributes.description, attributes.houses, attributes.spawns);

  Tiles tiles;
  Towns towns;
  Waypoints waypoints;

  for (auto &node : map_node.children) {
    if (node.type == NODETYPE_TILE_AREA) {
      parse_tile_area(node, items, [&](Coords &&coords, Tile &&tile) { tiles.emplace(std::move(coords), std::move(tile)); });
    } else if (node.type == NODETYPE_TOWNS) {
      parse_towns(node, [&](uint32_t id, Town &&town) { towns.insert_or_assign(id, std::move(town)); });
    } else if (node.type == NODETYPE_WAYPOINTS and header.version > 1) {
      parse_waypoints(node, [&](std::string &&name, Coords &&coords) { waypoints.insert_or_assign(std::move(name), std::move(coords)); });
    } else {
      throw std::invalid_argument(fmt::format("Unknown map node: {:d}", node.type));
    }
  }

  fmt::print("Loaded {:d} map tiles.\n", tiles.size());
  return {header, tiles, towns, waypoints};
}

} // namespace otbm
