#pragma once
#include <cstdint>
#include <memory>
#include <voxer/Dataset.hpp>
#include <voxer/TransferFunction.hpp>

namespace voxer {

struct Volume {
  std::shared_ptr<Dataset> dataset = nullptr;
  std::shared_ptr<TransferFunctionParams> tfcn = nullptr;
  std::array<float, 3> translate = {0.0f, 0.0f, 0.0f};
  std::array<float, 3> scale = {1.0f, 1.0f, 1.0f};
  std::array<float, 2> range;
  std::array<float, 3> clipBoxLower = {0.0f, 0.0f, 0.0f};
  std::array<float, 3> clipBoxUpper = {0.0f, 0.0f, 0.0f};
  bool render = false;
};

} // namespace voxer
