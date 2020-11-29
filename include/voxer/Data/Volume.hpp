#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <voxer/Data/StructuredGrid.hpp>
#include <voxer/Data/TransferFunction.hpp>

namespace voxer {

struct Volume {
  std::shared_ptr<StructuredGrid> dataset;
  std::shared_ptr<TransferFunction> tfcn;

  std::array<float, 3> spacing = {1.0f, 1.0f, 1.0f};

  std::array<float, 2> range = {0.0f, 100.0f};
  std::array<float, 3> translate = {0.0f, 0.0f, 0.0f};
  std::array<float, 3> scale = {1.0f, 1.0f, 1.0f};
  std::array<float, 3> clipBoxLower = {0.0f, 0.0f, 0.0f};
  std::array<float, 3> clipBoxUpper = {0.0f, 0.0f, 0.0f};
};

} // namespace voxer
