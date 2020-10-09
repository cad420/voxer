#pragma once
#include <vector>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

struct GradientFilter {
  std::vector<uint8_t> process(StructuredGrid &data);
};

} // namespace voxer