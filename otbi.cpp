#include "otbi.h"
#include "itemtype.h"
#include "stream.h"

#include <fmt/format.h>

namespace otbi {

namespace {

constexpr auto ROOT_ATTR_VERSION = 0x01;
constexpr auto CLIENT_VERSION_1098 = 57;

enum {
  ITEM_ATTR_FIRST = 0x10,
  ITEM_ATTR_SERVERID = ITEM_ATTR_FIRST,
  ITEM_ATTR_CLIENTID,
  ITEM_ATTR_NAME,
  ITEM_ATTR_DESCR,
  ITEM_ATTR_SPEED,
  ITEM_ATTR_SLOT,
  ITEM_ATTR_MAXITEMS,
  ITEM_ATTR_WEIGHT,
  ITEM_ATTR_WEAPON,
  ITEM_ATTR_AMU,
  ITEM_ATTR_ARMOR,
  ITEM_ATTR_MAGLEVEL,
  ITEM_ATTR_MAGFIELDTYPE,
  ITEM_ATTR_WRITEABLE,
  ITEM_ATTR_ROTATETO,
  ITEM_ATTR_DECAY,
  ITEM_ATTR_SPRITEHASH,
  ITEM_ATTR_MINIMAPCOLOR,
  ITEM_ATTR_07,
  ITEM_ATTR_08,
  ITEM_ATTR_LIGHT,

  // 1-byte aligned
  ITEM_ATTR_DECAY2,     // deprecated
  ITEM_ATTR_WEAPON2,    // deprecated
  ITEM_ATTR_AMU2,       // deprecated
  ITEM_ATTR_ARMOR2,     // deprecated
  ITEM_ATTR_WRITEABLE2, // deprecated
  ITEM_ATTR_LIGHT2,
  ITEM_ATTR_TOPORDER,
  ITEM_ATTR_WRITEABLE3, // deprecated

  ITEM_ATTR_WAREID,

  ITEM_ATTR_LAST,
};

auto type_from_group(otb::item_group group) {
  using namespace otb;

  switch (group) {
  case item_group::CONTAINER:
    return item_type::CONTAINER;

  case item_group::DOOR: // not used
    return item_type::DOOR;

  case item_group::MAGICFIELD: // not used
    return item_type::MAGICFIELD;

  case item_group::TELEPORT: // not used
    return item_type::TELEPORT;

  case item_group::NONE:
  case item_group::GROUND:
  case item_group::SPLASH:
  case item_group::FLUID:
  case item_group::CHARGES:
  case item_group::DEPRECATED:
    return item_type::NONE;

  default:
    throw std::invalid_argument(fmt::format("Invalid item group: {:d}", group));
  }
}

std::pair<uint32_t, uint32_t> read_version(otb::iterator &first, const otb::iterator &last) {
  auto length = read<uint16_t>(first, last);
  if (length != 140) {
    throw std::invalid_argument(fmt::format("Invalid data length for version info: expected 140, got {:d}", length));
  }

  auto major = read<uint32_t>(first, last);
  auto minor = read<uint32_t>(first, last);
  skip(first, last, 132);
  return std::make_pair(major, minor);
}

} // namespace

Items load(const std::string &filename) {
  auto loader = otb::load(filename, "OTBI");

  auto first = loader.begin();
  const auto last = loader.end();
  /*auto flags =*/read<uint32_t>(first, last); // unused
  auto attr = read<uint8_t>(first, last);

  uint32_t major = 0, minor = 0;
  if (attr == ROOT_ATTR_VERSION) {
    std::tie(major, minor) = read_version(first, last);
  }

  if (major == 0xFFFFFFFF) {
    fmt::print("[Warning] items.otb using generic client version.\n");
  } else if (major != 3) {
    throw std::invalid_argument("Old version detected, a newer version of items.otb is required.");
  } else if (minor < CLIENT_VERSION_1098) {
    throw std::invalid_argument("A newer version of items.otb is required.");
  }

  auto items = Items{};
  for (const auto &item_node : loader.children()) {
    auto first = item_node.props_begin;
    const auto last = item_node.props_end;

    auto flags = read<uint32_t>(first, last);

    std::string name, description;
    double weight = 0;
    uint16_t server_id = 0;
    uint16_t client_id = 0;
    uint16_t speed = 0;
    uint16_t max_items = 0;
    uint16_t rotate_to = 0;
    uint16_t read_only_id = 0;
    uint16_t max_text_length = 0;
    uint16_t ware_id = 0;
    uint8_t light_level = 0;
    uint8_t light_color = 0;
    uint8_t always_on_top_order = 0;

    while (first != last) {
      auto attr = read<uint8_t>(first, last);
      auto length = read<uint16_t>(first, last);

      switch (attr) {
      case ITEM_ATTR_SERVERID:
        if (length != sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid server ID attribute length: expected {:d}, got {:d}", sizeof(uint16_t), length));
        }

        server_id = read<uint16_t>(first, last);
        if (server_id > 30000 and server_id < 30100) {
          server_id -= 30000;
        }
        break;

      case ITEM_ATTR_CLIENTID:
        if (length != sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid client ID attribute length: expected {:d}, got {:d}", sizeof(uint16_t), length));
        }

        client_id = read<uint16_t>(first, last);
        break;

      case ITEM_ATTR_NAME: {
        if (length >= 128) {
          fmt::print("Unexpected item name length: {:d}", length);
        }

        name = read_string(first, last, length);
        break;
      }

      case ITEM_ATTR_DESCR: {
        if (length >= 128) {
          fmt::print("Unexpected item description length: {:d}", length);
        }

        description = read_string(first, last, length);
        break;
      }

      case ITEM_ATTR_SPEED:
        if (length != sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid speed attribute length: expected {:d}, got {:d}", sizeof(uint16_t), length));
        }

        speed = read<uint16_t>(first, last);
        break;

      case ITEM_ATTR_MAXITEMS:
        if (length != sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid max. items attribute length: expected {:d}, got {:d}", sizeof(uint16_t), length));
        }

        max_items = read<uint16_t>(first, last);
        break;

      case ITEM_ATTR_WEIGHT:
        if (length != sizeof(double)) {
          throw std::invalid_argument(fmt::format("Invalid weight attribute length: expected {:d}, got {:d}", sizeof(double), length));
        }

        weight = read<double>(first, last);
        break;

      case ITEM_ATTR_ROTATETO:
        if (length != sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid rotate to attribute length: expected {:d}, got {:d}", sizeof(uint16_t), length));
        }

        rotate_to = read<uint16_t>(first, last);
        break;

      case ITEM_ATTR_LIGHT2:
        if (length != 2 * sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid light2 attribute length: expected {:d}, got {:d}", 2 * sizeof(uint16_t), length));
        }

        light_level = static_cast<uint8_t>(read<uint16_t>(first, last));
        light_color = static_cast<uint8_t>(read<uint16_t>(first, last));
        break;

      case ITEM_ATTR_TOPORDER:
        if (length != sizeof(uint8_t)) {
          throw std::invalid_argument(fmt::format("Invalid top order attribute length: expected {:d}, got {:d}", sizeof(uint8_t), length));
        }

        always_on_top_order = read<uint8_t>(first, last);
        break;

      case ITEM_ATTR_WRITEABLE3:
        if (length != 2 * sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid writeable3 attribute length: expected {:d}, got {:d}", 2 * sizeof(uint16_t), length));
        }

        read_only_id = read<uint16_t>(first, last);
        max_text_length = read<uint16_t>(first, last);
        break;

      case ITEM_ATTR_WAREID:
        if (length != sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid ware ID attribute length: expected {:d}, got {:d}", sizeof(uint16_t), length));
        }

        ware_id = read<uint16_t>(first, last);
        break;

      case ITEM_ATTR_SPRITEHASH:
      case ITEM_ATTR_MINIMAPCOLOR:
      case ITEM_ATTR_07:
      case ITEM_ATTR_08:
        // not implemented
        skip(first, last, length);
        break;

      default:
        fmt::print("Unknown attribute {:d} length {:d} sid {:d} cid {:d}\n", attr, length, server_id, client_id);
        // skip unknown attributes
        skip(first, last, length);
        break;
      }
    }

    auto group = static_cast<otb::item_group>(item_node.type);
    auto type = type_from_group(group);

    items.emplace(server_id, otb::ItemType{name, description, weight, flags, server_id, client_id, speed, max_items, rotate_to, read_only_id, max_text_length,
                                           ware_id, light_level, light_color, always_on_top_order, group, type});
  }

  return items;
}

} // namespace otbi
