#pragma once
#include <cstdint>
#include <vector>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

struct StructuredGridHistogramMapper {
  uint32_t bins = 256;
  [[nodiscard]] std::vector<uint32_t> map(const StructuredGrid &dataset) const;
};

} // namespace voxer
