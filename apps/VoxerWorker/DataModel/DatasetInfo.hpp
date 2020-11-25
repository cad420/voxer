#pragma once
#include <cstdint>
#include <seria/object.hpp>
#include <string>

namespace voxer::remote {

using DatasetId = std::string;

struct DatasetInfo {
  DatasetId id;
  std::array<uint32_t, 3> dimensions{};
  std::vector<uint32_t> histogram{};
  std::array<float, 2> range{};
};

} // namespace voxer::remote

namespace seria {

template <> inline auto register_object<voxer::remote::DatasetInfo>() {
  using Object = voxer::remote::DatasetInfo;
  return std::make_tuple(
      member("id", &Object::id), member("dimensions", &Object::dimensions),
      member("histogram", &Object::histogram), member("range", &Object::range));
}

} // namespace seria
