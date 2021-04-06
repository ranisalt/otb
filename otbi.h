#pragma once

#include "itemtype.h"

#include <cstdint>
#include <string>
#include <tsl/robin_map.h>

namespace otbi {

using Items = tsl::robin_map<uint16_t, otb::ItemType>;

Items load(std::string_view filename);

} // namespace otbi