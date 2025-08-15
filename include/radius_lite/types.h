#pragma once

#include <array>
#include <cstdint>

namespace radius_lite
{
  using Auth = std::array<uint8_t, 16>;
  using ByteArray = std::vector<uint8_t>;
}
