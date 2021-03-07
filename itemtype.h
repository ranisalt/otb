#pragma once

#include <cstdint>
#include <string>
#include <tsl/robin_map.h>
#include <variant>

namespace otb {

namespace item_flag {} // namespace item_flag

enum class item_group : uint8_t {
  NONE,

  GROUND,
  CONTAINER,
  WEAPON,     // deprecated
  AMMUNITION, // deprecated
  ARMOR,      // deprecated
  CHARGES,
  TELEPORT,   // deprecated
  MAGICFIELD, // deprecated
  WRITEABLE,  // deprecated
  KEY,        // deprecated
  SPLASH,
  FLUID,
  DOOR, // deprecated
  DEPRECATED,

  LAST
};

enum class item_type : uint8_t { NONE, DEPOT, MAILBOX, TRASHHOLDER, CONTAINER, DOOR, MAGICFIELD, TELEPORT, BED, KEY, RUNE, LAST };

struct ItemType {
  enum {
    BLOCK_SOLID = 1 << 0,
    BLOCK_PROJECTILE = 1 << 1,
    BLOCK_PATHFIND = 1 << 2,
    HAS_HEIGHT = 1 << 3,
    USEABLE = 1 << 4,
    PICKUPABLE = 1 << 5,
    MOVEABLE = 1 << 6,
    STACKABLE = 1 << 7,
    FLOORCHANGEDOWN = 1 << 8,   // unused
    FLOORCHANGENORTH = 1 << 9,  // unused
    FLOORCHANGEEAST = 1 << 10,  // unused
    FLOORCHANGESOUTH = 1 << 11, // unused
    FLOORCHANGEWEST = 1 << 12,  // unused
    ALWAYSONTOP = 1 << 13,
    READABLE = 1 << 14,
    ROTATABLE = 1 << 15,
    HANGABLE = 1 << 16,
    VERTICAL = 1 << 17,
    HORIZONTAL = 1 << 18,
    CANNOTDECAY = 1 << 19, // unused
    ALLOWDISTREAD = 1 << 20,
    UNUSED = 1 << 21,        // unused
    CLIENTCHARGES = 1 << 22, /* deprecated */
    LOOKTHROUGH = 1 << 23,
    ANIMATION = 1 << 24,
    FULLTILE = 1 << 25, // unused
    FORCEUSE = 1 << 26,
  };

public:
  ItemType() = default;
  ItemType(std::string name, std::string description, double weight, uint32_t flags, uint16_t server_id, uint16_t client_id, uint16_t speed, uint16_t max_items,
           uint16_t rotate_to, uint16_t read_only_id, uint16_t max_text_length, uint16_t ware_id, uint8_t light_level, uint8_t light_color,
           uint8_t always_on_top_order, item_group group, item_type type)
      : name_{std::move(name)}, description{std::move(description)}, weight{weight}, flags{flags}, server_id{server_id}, client_id{client_id}, speed{speed},
        max_items{max_items}, rotate_to{rotate_to}, read_only_id{read_only_id}, max_text_length{max_text_length}, ware_id{ware_id}, light_level{light_level},
        light_color{light_color}, always_on_top_order{always_on_top_order}, group{group}, type{type} {}

  auto charges() const { return charges_; }

  bool block_solid() const { return flags & BLOCK_SOLID; }
  bool block_projectile() const { return flags & BLOCK_PROJECTILE; }
  bool block_path_find() const { return flags & BLOCK_PATHFIND; }
  bool has_height() const { return flags & HAS_HEIGHT; }
  bool useable() const { return flags & USEABLE; }
  bool pickupable() const { return flags & PICKUPABLE; }
  bool moveable() const { return flags & MOVEABLE; }
  bool stackable() const { return flags & STACKABLE; }
  bool always_on_top() const { return flags & ALWAYSONTOP; }
  bool is_vertical() const { return flags & VERTICAL; }
  bool is_horizontal() const { return flags & HORIZONTAL; }
  bool hangable() const { return flags & HANGABLE; }
  bool allow_dist_read() const { return flags & ALLOWDISTREAD; }
  bool rotatable() const { return flags & ROTATABLE; }
  bool readable() const { return flags & READABLE; }
  bool look_through() const { return flags & LOOKTHROUGH; }
  bool is_animation() const { return flags & ANIMATION; }

  bool is_ground_tile() const { return group == item_group::GROUND; }
  bool is_container() const { return group == item_group::CONTAINER; }
  bool is_splash() const { return group == item_group::SPLASH; }
  bool is_fluid_container() const { return group == item_group::FLUID; }

  auto &name() const { return name_; }

private:
  std::string name_, description;

  double weight;

  uint32_t flags;
  uint16_t charges_;

  uint16_t server_id;
  uint16_t client_id;
  uint16_t speed;
  uint16_t max_items;
  uint16_t rotate_to;
  uint16_t read_only_id;
  uint16_t max_text_length;
  uint16_t ware_id;

  uint8_t light_level;
  uint8_t light_color;
  uint8_t always_on_top_order;

  item_group group;
  item_type type;
};

struct Item {
  Item() = default;
  explicit Item(const ItemType *type) : type{type}, charges{type->charges()} {}

  void subtype(uint8_t value) {
    if (type->is_fluid_container() or type->is_splash()) {
      fluid_type = value;
    } else if (type->stackable()) {
      count = value;
    } else if (type->charges() != 0) {
      charges = value;
    } else {
      count = value;
    }
  }

  using attribute = std::variant<std::string, int64_t, double, bool>;

  const ItemType *type;
  tsl::robin_map<std::string, attribute> custom_attributes;
  std::string text;
  std::string writer;
  std::string description;
  std::string name;
  std::string article;
  std::string plural_name;
  uint32_t written_at = 0;
  uint32_t weight;
  int32_t duration = 0;
  int32_t attack = 0;
  int32_t defense = 0;
  int32_t extra_defense = 0;
  int32_t armor = 0;
  int32_t decay_to = 0;
  uint16_t fluid_type = 0;
  uint16_t count = 0;
  uint16_t charges;
  uint16_t action_id = 0;
  uint16_t unique_id = 0;
  uint16_t wrap_id = 0;
  uint8_t shoot_range = 0;
  uint8_t store_item = 0;
  int8_t hit_chance = 0;
};

} // namespace otb