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

class ItemType {
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
  ItemType(std::string name, std::string description, double weight, uint32_t flags, uint16_t server_id, uint16_t client_id, uint16_t speed, uint16_t max_items,
           uint16_t rotate_to, uint16_t read_only_id, uint16_t max_text_length, uint16_t ware_id, uint16_t light_level, uint16_t light_color,
           uint8_t always_on_top_order, item_group group, item_type type)
      : name_{std::move(name)}, description{std::move(description)}, weight{weight}, flags{flags}, server_id{server_id}, client_id{client_id}, speed{speed},
        max_items{max_items}, rotate_to{rotate_to}, read_only_id{read_only_id}, max_text_length{max_text_length}, ware_id{ware_id}, light_level{light_level},
        light_color{light_color}, always_on_top_order{always_on_top_order}, group{group}, type{type} {}

  auto charges() const { return charges_; }

  bool block_solid() const { return (flags & BLOCK_SOLID) != 0; }
  bool block_projectile() const { return (flags & BLOCK_PROJECTILE) != 0; }
  bool block_path_find() const { return (flags & BLOCK_PATHFIND) != 0; }
  bool has_height() const { return (flags & HAS_HEIGHT) != 0; }
  bool useable() const { return (flags & USEABLE) != 0; }
  bool pickupable() const { return (flags & PICKUPABLE) != 0; }
  bool moveable() const { return (flags & MOVEABLE) != 0; }
  bool stackable() const { return (flags & STACKABLE) != 0; }
  bool always_on_top() const { return (flags & ALWAYSONTOP) != 0; }
  bool is_vertical() const { return (flags & VERTICAL) != 0; }
  bool is_horizontal() const { return (flags & HORIZONTAL) != 0; }
  bool hangable() const { return (flags & HANGABLE) != 0; }
  bool allow_dist_read() const { return (flags & ALLOWDISTREAD) != 0; }
  bool rotatable() const { return (flags & ROTATABLE) != 0; }
  bool readable() const { return (flags & READABLE) != 0; }
  bool look_through() const { return (flags & LOOKTHROUGH) != 0; }
  bool is_animation() const { return (flags & ANIMATION) != 0; }

  bool is_ground_tile() const { return group == item_group::GROUND; }
  bool is_container() const { return group == item_group::CONTAINER; }
  bool is_splash() const { return group == item_group::SPLASH; }
  bool is_fluid_container() const { return group == item_group::FLUID; }

  auto id() const { return server_id; }

  void name(std::string name) { name_ = std::move(name); }
  auto &name() const { return name_; }

  void article(std::string article) { article_ = std::move(article); }
  auto &article() const { return article_; }

  void plural_name(std::string plural_name) { plural_name_ = std::move(plural_name); }
  auto &plural_name() const { return plural_name_; }

private:
  std::string name_;
  std::string description;
  std::string article_ = {};
  std::string plural_name_ = {};

  double weight;

  uint32_t flags;
  uint16_t charges_ = 0;

  uint16_t server_id;
  uint16_t client_id;
  uint16_t speed;
  uint16_t max_items;
  uint16_t rotate_to;
  uint16_t read_only_id;
  uint16_t max_text_length;
  uint16_t ware_id;
  uint16_t light_level;
  uint16_t light_color;

  uint8_t always_on_top_order;

  item_group group;
  item_type type;
};

struct Item {
  explicit Item(const ItemType *type) : type{type}, charges{type->charges()} {}

  Item(const Item&) = default;
  Item& operator=(const Item&) = default;

  void subtype(uint8_t value) {
    if (type->is_fluid_container() or type->is_splash()) {
      fluid_type = value;
    } else if (type->stackable() or type->charges() == 0) {
      count = value;
    } else {
      charges = value;
    }
  }

  using attribute = std::variant<std::string, int64_t, double, bool>;

  const ItemType *type;
  tsl::robin_map<std::string, attribute> custom_attributes = {};
  std::string text = {};
  std::string writer = {};
  std::string description = {};
  std::string name = {};
  std::string article = {};
  std::string plural_name = {};
  uint32_t written_at = 0;
  uint32_t weight = 0;
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
  uint8_t hit_chance = 0;
};

} // namespace otb