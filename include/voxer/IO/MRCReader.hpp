#pragma once
#include <array>
#include <fstream>
#include <memory>
#include <string>
#include <voxer/Data/StructuredGrid.hpp>
#include <voxer/IO/AbstractDatasetReader.hpp>

namespace voxer {

struct MRCReader : public AbstractDatasetReader {
  std::ifstream fs;

  explicit MRCReader(const std::string &filepath);
  auto load() -> std::unique_ptr<StructuredGrid> override;
  auto load_region(const std::array<uint16_t, 3> &begin,
                   const std::array<uint16_t, 3> &end)
      -> std::unique_ptr<StructuredGrid>;
};

} // namespace voxer