#pragma once
#include <voxer/Dataset.hpp>

namespace voxer {

auto differ(const Dataset &lfh, const Dataset &rhs) -> Dataset;

}