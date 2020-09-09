#pragma once
#include <memory>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

struct ContrastFilter {
  std::unique_ptr<StructuredGrid> process(StructuredGrid *volume_data) const;

  float brightness = 1.0f;
  float contrast = 5.0f;
};

} // namespace voxer