#pragma once
#include <cstdint>
#include <vector>

namespace voxer {

auto differ(const std::vector<uint8_t> &lfh, const std::vector<uint8_t> &rhs)
    -> std::vector<uint8_t>;

}