#pragma once
#include <cstdint>
#include <vector>
#include <voxer/Dataset.hpp>

namespace voxer {

auto calculate_histogram(const Dataset &dataset) -> std::vector<uint32_t>;

}
