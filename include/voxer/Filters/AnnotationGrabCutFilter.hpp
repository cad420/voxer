#pragma once
#include <vector>
#include <voxer/Data/Annotation.hpp>
#include <voxer/Data/Image.hpp>

namespace voxer {

struct AnnotationGrabCutFilter {
  int iteration = 2;
  float threshold = 100.0f;

  [[nodiscard]] std::vector<Annotation>
  process(const Image &image, const std::vector<Annotation> &annotations) const;
};

} // namespace voxer