#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace voxer {

struct Dataset {
  std::vector<uint8_t> buffer;
  std::array<uint32_t, 3> dimensions;
  enum class ValueType { FLOAT, UCHAR };
  ValueType type = ValueType::UCHAR;
};

} // namespace voxer
