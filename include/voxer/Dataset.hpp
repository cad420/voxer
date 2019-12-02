#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace voxer {

using Buffer = std::vector<uint8_t>;

struct DatasetVariable {
  std::string name;
  std::vector<Buffer> timesteps;
};

struct Dataset {
  std::string name;
  std::map<std::string, DatasetVariable> variables;
  std::array<uint32_t, 3> dimensions;
  enum class ValueType { FLOAT, UCHAR };
  ValueType type = ValueType::UCHAR;
  uint8_t type_size = sizeof(uint8_t);
};

} // namespace voxer
