#pragma once
#include "DataModel/StructuredGrid.hpp"
#include <future>
#include <string>

namespace voxer::remote {

auto query_dataset(const std::string &address, const std::string &id)
    -> std::promise<Dataset>;

}