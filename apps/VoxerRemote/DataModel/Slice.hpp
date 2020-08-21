#pragma once
#include <seria/utils.hpp>
#include <string>
#include <voxer/Slice.hpp>

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

} // namespace seria
