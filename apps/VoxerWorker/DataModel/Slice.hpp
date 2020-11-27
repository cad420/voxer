#pragma once
#include "DataModel/DatasetInfo.hpp"
#include <seria/deserialize.hpp>
#include <seria/object.hpp>
#include <seria/serialize.hpp>
#include <string>
#include <voxer/Data/Slice.hpp>

namespace voxer::remote {

struct Slice {
  DatasetId dataset;
  StructuredGrid::Axis axis = StructuredGrid::Axis::X;
  uint32_t index = 0;
};

} // namespace voxer::remote

namespace seria {

template <>
inline rapidjson::Document serialize(const voxer::StructuredGrid::Axis &axis) {
  std::string c;
  c += static_cast<char>('x' + static_cast<int>(axis));
  rapidjson::Document json(rapidjson::kStringType);
  json.SetString(c.c_str(), json.GetAllocator());
  return json;
}

template <>
inline void deserialize(voxer::StructuredGrid::Axis &axis,
                        const rapidjson::Value &json) {
  if (!json.IsString()) {
    throw std::runtime_error("invalid type, should be string");
  }

  axis = static_cast<voxer::StructuredGrid::Axis>(*(json.GetString()) - 'x');
}

template <> inline auto register_object<voxer::remote::Slice>() {
  using Slice = voxer::remote::Slice;
  using Axis = voxer::StructuredGrid::Axis;
  return std::make_tuple(member("dataset", &Slice::dataset),
                         member("axis", &Slice::axis),
                         member("index", &Slice::index));
}

} // namespace seria
