#pragma once
#include <seria/object.hpp>
#include <string>
#include <voxer/Data/Slice.hpp>

namespace voxer::remote {

struct Slice {
  uint32_t dataset = 0;
  StructuredGrid::Axis axis = StructuredGrid::Axis::X;
  uint32_t index = 0;
};

} // namespace voxer::remote

namespace seria {

template <> inline auto register_object<voxer::Annotation>() {
  using Annotation = voxer::Annotation;
  return std::make_tuple(member("type", &Annotation::type),
                         member("coordinates", &Annotation::coordinates),
                         member("tag", &Annotation::tag),
                         member("comment", &Annotation::comment));
}

template <> inline auto register_object<voxer::remote::Slice>() {
  using Slice = voxer::remote::Slice;
  using Axis = voxer::StructuredGrid::Axis;
  return std::make_tuple(member("dataset", &Slice::dataset),
                         member<Slice, Axis, const char *>(
                             "axis", &Slice::axis, nullptr,
                             [](const char *const &v) -> Axis {
                               if (std::strcmp(v, "x") == 0) {
                                 return Axis::X;
                               }

                               if (std::strcmp(v, "y") == 0) {
                                 return Axis::Y;
                               }

                               if (std::strcmp(v, "z") == 0) {
                                 return Axis::Z;
                               }

                               throw std::runtime_error("invalid value");
                             },
                             [](const Axis &axis) -> const char * {
                               switch (axis) {
                               case Axis::X:
                                 return "x";
                               case Axis::Y:
                                 return "y";
                               case Axis::Z:
                                 return "z";
                               default:
                                 return "";
                               }
                             }),
                         member("index", &Slice::index));
}

} // namespace seria
