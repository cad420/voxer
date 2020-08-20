#pragma once
#include "voxer/Dataset.hpp"

namespace voxer {

class AbstractDatasetReader {
public:
  virtual ~AbstractDatasetReader() = default;

  virtual auto load() -> Dataset = 0;
};

} // namespace voxer
