#pragma once
#include <cstdint>
#include <vector>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

auto calculate_histogram(const StructuredGrid &dataset) -> std::vector<uint32_t>;

}
