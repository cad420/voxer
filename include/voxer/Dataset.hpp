#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <voxer/Image.hpp>

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

  // TODO: byte-wise compare?
  bool operator==(const VolumeInfo &another) {
    if (this->value_type != another.value_type) {
      return false;
    }

    for (int_fast8_t i = 0; i < 3; i++) {
      if (this->dimensions[i] != another.dimensions[i]) {
        return false;
      }
    }

    return true;
  }

  bool operator!=(const VolumeInfo &another) { return !(*this == another); }
};

struct Dataset {
  std::string id = "";
  std::string name = "";
  std::string variable = "";
  uint32_t timestep = 0;
  VolumeInfo info{};
  std::vector<uint8_t> buffer{};
  std::vector<uint32_t> histogram{};

  enum struct Axis : uint_fast8_t { X, Y, Z };

  template <typename T> auto get() -> T * {
    return reinterpret_cast<T *>(buffer.data());
  }

  template <typename T> auto get(uint32_t index) -> T {
    return reinterpret_cast<T *>(buffer.data())[index];
  }

  [[nodiscard]] auto get_slice(Axis axis, uint32_t slice) const -> Image;

  [[nodiscard]] auto serialize() const -> std::string;

  [[nodiscard]] static auto Load(const char *path) -> Dataset;
};

} // namespace voxer
