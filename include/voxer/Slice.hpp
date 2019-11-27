#pragma once
#include <array>
#include <memory>
#include <voxer/Volume.hpp>

namespace voxer {

struct Slice {
  std::shared_ptr<Volume> volume;
  std::array<float, 4> coef;
};

} // namespace voxer