#pragma once
#include <seria/object.hpp>
#include <voxer/Data/Annotation.hpp>

namespace seria {

template <> inline auto register_object<voxer::Annotation>() {
  using Annotation = voxer::Annotation;
  return std::make_tuple(member("id", &Annotation::id, 0u),
                         member("type", &Annotation::type, std::string("Rect")),
                         member("coordinates", &Annotation::coordinates),
                         member("tag", &Annotation::label, std::string("")),
                         member("bbox", &Annotation::bbox, {0, 0, 0, 0}));
}

} // namespace seria
