#pragma once
#include "voxer/Dataset.hpp"
#include <array>
#include <fstream>
#include <string>
#include "Datasets/Readers/AbstractDatasetReader.hpp"

namespace voxer {

struct MRCReader : public AbstractDatasetReader {
  std::ifstream fs;

  explicit MRCReader(const std::string &filepath);
  auto load() -> Dataset override;
  auto load_region(const std::array<uint16_t, 3> &begin,
                   const std::array<uint16_t, 3> &end) -> Dataset;
};

} // namespace voxer