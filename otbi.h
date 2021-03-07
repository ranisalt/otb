#pragma once

#include "itemtype.h"

#include <cstdint>
#include <string>
#include <tsl/robin_map.h>

namespace otbi {

using Items = tsl::robin_map<uint16_t, otb::ItemType>;

Items load(const std::string &filename);

} // namespace otbi