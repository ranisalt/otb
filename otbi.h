#pragma once

#include "itemtype.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace otbi {

using Items = std::unordered_map<uint16_t, otb::ItemType>;

Items load(const std::string &filename);

} // namespace otbi