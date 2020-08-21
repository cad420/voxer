#pragma once
#include <memory>
#include <rapidjson/document.h>
#include <voxer/Data/Color.hpp>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

struct Isosurface {
  StructuredGrid dataset;
  float value = 0.0f;
  RGBColor color;
};

} // namespace voxer
