#pragma once
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

class AbstractDatasetReader {
public:
  virtual ~AbstractDatasetReader() = default;

  virtual auto load() -> StructuredGrid = 0;
};

} // namespace voxer
