#pragma once
#include <voxer/Data/Image.hpp>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

struct EqualizeFilter {
  int n = 5;
  int min = 100;
  int max = 255;

  void process(Image *image) const;
  void process(StructuredGrid *volume_data) const;
};

} // namespace voxer