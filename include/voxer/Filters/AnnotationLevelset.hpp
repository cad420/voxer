#pragma once
#include <vector>
#include <voxer/Data/Annotation.hpp>
#include <voxer/Data/Image.hpp>

namespace voxer {

struct AnnotationLevelSetFilter {
  int inner_iteration = 5;
  int outer_iteration = 10;

  [[nodiscard]] Annotation process(const Annotation &annotations,
                                   const Image &image) const;
};

} // namespace voxer