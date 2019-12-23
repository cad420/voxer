#pragma once
#include "voxer/Dataset.hpp"
#include <array>
#include <string>

namespace voxer {

struct MRCReader {
  std::fstream fs;

  explicit MRCReader(const std::string &filepath);
  auto load() -> Dataset;
  auto load_region(const std::array<uint16_t, 3> &begin,
                   const std::array<uint16_t, 3> &end) -> Dataset;
};

} // namespace voxer