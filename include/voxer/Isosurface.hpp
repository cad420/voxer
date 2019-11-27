#pragma once
#include <memory>
#include <voxer/Volume.hpp>

namespace voxer {

struct Isosurface {
  float value;
  std::shared_ptr<Volume> volume;
};

} // namespace voxer
