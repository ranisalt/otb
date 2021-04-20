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

auto read_version(otb::iterator &first, const otb::iterator &last) {
  constexpr auto VERSION_INFO_SIZE = 140;

  auto length = read<uint16_t>(first, last);
  if (length != VERSION_INFO_SIZE) {
    throw std::invalid_argument(fmt::format("Invalid data length for version info: expected 140, got {:d}", length));
  }

  auto major = read<uint32_t>(first, last);
  auto minor = read<uint32_t>(first, last);
  skip(first, last, VERSION_INFO_SIZE - 2 * sizeof(uint32_t));
  return std::make_pair(major, minor);
}

} // namespace

Items load(std::string_view filename) {
  auto loader = otb::load(filename, "OTBI");

  auto root_begin = loader.begin();
  const auto root_end = loader.end();
  /*auto flags =*/read<uint32_t>(root_begin, root_end); // unused
  auto root_attr = read<uint8_t>(root_begin, root_end);

  uint32_t major = 0;
  uint32_t minor = 0;
  if (root_attr == ROOT_ATTR_VERSION) {
    std::tie(major, minor) = read_version(root_begin, root_end);
  }

  if (major == std::numeric_limits<uint32_t>::max()) {
    fmt::print("[Warning] items.otb using generic client version.\n");
  } else if (major != 3) {
    throw std::invalid_argument("Old version detected, a newer version of items.otb is required.");
  } else if (minor < CLIENT_VERSION_1098) {
    throw std::invalid_argument("A newer version of items.otb is required.");
  }

  auto items = Items{};
  for (const auto &item_node : loader.children()) {
    auto node_begin = item_node.props_begin;
    const auto node_end = item_node.props_end;

    auto flags = read<uint32_t>(node_begin, node_end);

    constexpr auto MAX_TEXT_LENGTH = 128;
    std::string name;
    std::string description;
    double weight = 0;
    uint16_t server_id = 0;
    uint16_t client_id = 0;
    uint16_t speed = 0;
    uint16_t max_items = 0;
    uint16_t rotate_to = 0;
    uint16_t read_only_id = 0;
    uint16_t max_text_length = 0;
    uint16_t ware_id = 0;
    uint16_t light_level = 0;
    uint16_t light_color = 0;
    uint8_t always_on_top_order = 0;

    while (node_begin != node_end) {
      auto attr = read<uint8_t>(node_begin, node_end);
      auto length = read<uint16_t>(node_begin, node_end);

      switch (attr) {
      case ITEM_ATTR_SERVERID:
        if (length != sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid server ID attribute length: expected {:d}, got {:d}", sizeof(uint16_t), length));
        }

        static constexpr auto ID_RESERVED = 30000;
        static constexpr auto ID_RESERVED_SIZE = 100;
        server_id = read<uint16_t>(node_begin, node_end);
        if (server_id > ID_RESERVED and server_id < ID_RESERVED + ID_RESERVED_SIZE) {
          server_id -= ID_RESERVED;
        }
        break;

      case ITEM_ATTR_CLIENTID:
        if (length != sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid client ID attribute length: expected {:d}, got {:d}", sizeof(uint16_t), length));
        }

        client_id = read<uint16_t>(node_begin, node_end);
        break;

      case ITEM_ATTR_NAME: {
        if (length >= MAX_TEXT_LENGTH) {
          fmt::print("[Warning] Unexpected item name length: {:d}", length);
        }

        name = read_string(node_begin, node_end, length);
        break;
      }

      case ITEM_ATTR_DESCR: {
        if (length >= MAX_TEXT_LENGTH) {
          fmt::print("[Warning] Unexpected item description length: {:d}", length);
        }

        description = read_string(node_begin, node_end, length);
        break;
      }

      case ITEM_ATTR_SPEED:
        if (length != sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid speed attribute length: expected {:d}, got {:d}", sizeof(uint16_t), length));
        }

        speed = read<uint16_t>(node_begin, node_end);
        break;

      case ITEM_ATTR_MAXITEMS:
        if (length != sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid max. items attribute length: expected {:d}, got {:d}", sizeof(uint16_t), length));
        }

        max_items = read<uint16_t>(node_begin, node_end);
        break;

      case ITEM_ATTR_WEIGHT:
        if (length != sizeof(double)) {
          throw std::invalid_argument(fmt::format("Invalid weight attribute length: expected {:d}, got {:d}", sizeof(double), length));
        }

        weight = read<double>(node_begin, node_end);
        break;

      case ITEM_ATTR_ROTATETO:
        if (length != sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid rotate to attribute length: expected {:d}, got {:d}", sizeof(uint16_t), length));
        }

        rotate_to = read<uint16_t>(node_begin, node_end);
        break;

      case ITEM_ATTR_LIGHT2:
        if (length != 2 * sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid light2 attribute length: expected {:d}, got {:d}", 2 * sizeof(uint16_t), length));
        }

        light_level = read<uint16_t>(node_begin, node_end);
        light_color = read<uint16_t>(node_begin, node_end);
        break;

      case ITEM_ATTR_TOPORDER:
        if (length != sizeof(uint8_t)) {
          throw std::invalid_argument(fmt::format("Invalid top order attribute length: expected {:d}, got {:d}", sizeof(uint8_t), length));
        }

        always_on_top_order = read<uint8_t>(node_begin, node_end);
        break;

      case ITEM_ATTR_WRITEABLE3:
        if (length != 2 * sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid writeable3 attribute length: expected {:d}, got {:d}", 2 * sizeof(uint16_t), length));
        }

        read_only_id = read<uint16_t>(node_begin, node_end);
        max_text_length = read<uint16_t>(node_begin, node_end);
        break;

      case ITEM_ATTR_WAREID:
        if (length != sizeof(uint16_t)) {
          throw std::invalid_argument(fmt::format("Invalid ware ID attribute length: expected {:d}, got {:d}", sizeof(uint16_t), length));
        }

        ware_id = read<uint16_t>(node_begin, node_end);
        break;

      case ITEM_ATTR_SPRITEHASH:
      case ITEM_ATTR_MINIMAPCOLOR:
      case ITEM_ATTR_07:
      case ITEM_ATTR_08:
        // not implemented
        skip(node_begin, node_end, length);
        break;

      default:
        fmt::print("[Warning] Unknown attribute {:d} length {:d} sid {:d} cid {:d}\n", attr, length, server_id, client_id);
        // skip unknown attributes
        skip(node_begin, node_end, length);
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
