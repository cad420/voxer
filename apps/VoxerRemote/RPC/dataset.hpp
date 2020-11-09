#pragma once
#include "DataModel/StructuredGrid.hpp"
#include <string>

namespace voxer::remote {

auto get_dataset_info(const std::string &address, const std::string &id)
    -> Dataset;

}