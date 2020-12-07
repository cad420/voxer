#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <voxer/Data/Image.hpp>

namespace voxer {

enum class VolumeType : uint8_t {
  SCALAR,
  VECTOR,
  TENSOR,
};

enum class ValueType : uint8_t {
  FLOAT = 0,
  DOUBLE = 1,
  UINT8 = 2,
  UINT16 = 3,
  UINT32 = 4,
  UINT64 = 5,
  INT8 = 6,
  INT16 = 7,
  INT32 = 8,
  INT64 = 9,
};

struct VolumeInfo {
  // only support scalar field
  // VolumeType type = VolumeType::SCALAR;
  ValueType value_type = ValueType::UINT8;
  std::array<uint32_t, 3> dimensions{0, 0, 0};
  uint32_t component = 1;
  [[nodiscard]] auto value_type_size() const -> uint8_t {
    return static_cast<uint8_t>(value_type);
  }
  [[nodiscard]] auto voxel_count() const -> uint64_t {
    return dimensions[0] * dimensions[1] * dimensions[2];
  }
  [[nodiscard]] auto byte_count() const -> uint64_t {
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

struct StructuredGrid : public std::enable_shared_from_this<StructuredGrid> {
  std::string name;
  VolumeInfo info{};
  std::array<float, 2> original_range;
  std::vector<uint8_t> buffer{};

  enum struct Axis { X = 0, Y = 1, Z = 2 };

  [[nodiscard]] auto get_slice(Axis axis, uint32_t slice) const -> Image;

  [[nodiscard]] auto get_histogram() const -> std::vector<uint32_t>;

  [[nodiscard]] static auto Load(const char *path)
      -> std::shared_ptr<StructuredGrid>;

  StructuredGrid operator-(const StructuredGrid &rhs) const;
  
  static Axis get_axis(const char *str);
};

} // namespace voxer
