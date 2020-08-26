#pragma once
#include <seria/utils.hpp>
#include <string>
#include <voxer/Data/Slice.hpp>

namespace voxer::remote {

struct Slice : public AnnotatedSliceInfo {
  std::string dataset;
};

} // namespace voxer::remote

namespace seria {

template <> inline auto registerObject<voxer::Annotation>() {
  using Annotation = voxer::Annotation;
  return std::make_tuple(member("type", &Annotation::type),
                         member("coordinates", &Annotation::coordinates),
                         member("tag", &Annotation::tag),
                         member("comment", &Annotation::comment));
}

template <> inline auto registerObject<voxer::remote::Slice>() {
  using Slice = voxer::remote::Slice;
  return std::make_tuple(member("dataset", &Slice::dataset),
                         member("axis", &Slice::axis),
                         member("index", &Slice::index),
                         member("annotations", &Slice::annotations));
}

template <typename T>
std::enable_if_t<std::is_same_v<voxer::StructuredGrid::Axis, T>>
deserialize(T &obj, const rapidjson::Value &value) {
  using Axis = voxer::StructuredGrid::Axis;

  rapidjson::Document json(rapidjson::kStringType);
  if (!value.IsString()) {
    throw std::runtime_error("wrong type");
  }

  std::string str = value.GetString();
  if (str == "x") {
    obj = Axis::X;
  } else if (str == "y") {
    obj = Axis::Y;
  } else if (str == "z") {
    obj = Axis::Z;
  } else {
    throw std::runtime_error("invalid value");
  }
}

template <typename T>
std::enable_if_t<std::is_same_v<T, voxer::StructuredGrid::Axis>,
                 rapidjson::Document>
serialize(const T &obj) {
  using Axis = voxer::StructuredGrid::Axis;

  rapidjson::Document json(rapidjson::kStringType);
  switch (obj) {
  case Axis::X:
    json.SetString("x", 1);
    break;
  case Axis::Y:
    json.SetString("y", 1);
    break;
  case Axis::Z:
    json.SetString("z", 1);
    break;
  default:
    break;
  }

  return json;
}
} // namespace seria
