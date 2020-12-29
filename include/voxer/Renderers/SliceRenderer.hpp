#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

class SliceRenderer {
public:
  using MarkType = std::pair<StructuredGrid *, std::array<float, 3>>;

  void set_dataset(StructuredGrid *dataset);
  void add_mark(StructuredGrid *dataset, const std::string &hex_color);
  void add_mark(StructuredGrid *dataset, const std::array<float, 3> &color);
  [[nodiscard]] Image render(StructuredGrid::Axis axis, uint32_t slice) const;

private:
  StructuredGrid *m_dataset = nullptr;
  VolumeInfo m_info{};

  std::vector<MarkType> m_marks{};
};

} // namespace voxer