#pragma once
#include <memory>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

class AbstractDatasetReader {
public:
  virtual ~AbstractDatasetReader() = default;

  virtual auto load() -> std::unique_ptr<StructuredGrid> = 0;
};

} // namespace voxer
