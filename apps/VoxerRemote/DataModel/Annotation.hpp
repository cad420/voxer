#pragma once
#include <seria/object.hpp>
#include <voxer/Data/Annotation.hpp>

namespace seria {

template <> inline auto register_object<voxer::Annotation>() {
  using Annotation = voxer::Annotation;
  return std::make_tuple(member("type", &Annotation::type),
                         member("coordinates", &Annotation::coordinates),
                         member("tag", &Annotation::label));
}

} // namespace seria
