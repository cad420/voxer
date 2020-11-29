#pragma once
#include <memory>
#include <voxer/Data/Color.hpp>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

struct Isosurface {
  std::shared_ptr<StructuredGrid> dataset;
  float value = 0.0f;
  RGBColor color;
};

} // namespace voxer
