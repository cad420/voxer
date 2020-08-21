#pragma once
#include "Datasets/Readers/AbstractDatasetReader.hpp"
#include "voxer/Dataset.hpp"
#include <array>
#include <fstream>

namespace voxer {

struct RawReader : public AbstractDatasetReader {
  std::ifstream fs;
  std::array<uint16_t, 3> dimensions;
  ValueType value_type;

  explicit RawReader(const char *json_path);
  RawReader(const std::string &filepath,
            const std::array<uint16_t, 3> &dimensions, ValueType value_type);

  auto load() -> Dataset override;
  auto load_region(const std::array<uint16_t, 3> &begin,
                   const std::array<uint16_t, 3> &end) -> Dataset;
};

} // namespace voxer