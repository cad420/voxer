#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace voxer {

enum class VolumeType : uint8_t {
  SCALAR,
  VECTOR,
  TENSOR,
};

enum class ValueType : uint8_t {
  FLOAT = 4,
  DOUBLE = 8,
  UINT8 = 1,
  UINT16 = 2,
  UINT32 = 4,
  UINT64 = 8
};

struct VolumeInfo {
  // only support scalar field
  // VolumeType type = VolumeType::SCALAR;
  ValueType value_type = ValueType::UINT8;
  std::array<uint16_t, 3> dimensions{0, 0, 0};
  auto value_type_size() -> uint8_t { return static_cast<uint8_t>(value_type); }
  auto voxel_count() -> uint64_t {
    return dimensions[0] * dimensions[1] * dimensions[2];
  }
  auto byte_count() -> uint64_t {
    return this->voxel_count() * this->value_type_size();
  }
};

struct Dataset {
  std::string id = "";
  VolumeInfo info{};
  std::vector<uint8_t> buffer{};

  template <typename T> auto get() -> T * {
    return reinterpret_cast<T *>(buffer.data());
  }

  template <typename T> auto get(uint32_t index) -> T {
    return reinterpret_cast<T *>(buffer.data())[index];
  }
};

} // namespace voxer
