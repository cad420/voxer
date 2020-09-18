#pragma once
#include <vector>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

struct GradientFilter {
  std::vector<float> process(StructuredGrid &data);
};

} // namespace voxer