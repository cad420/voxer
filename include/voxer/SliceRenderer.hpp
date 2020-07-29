#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <voxer/Dataset.hpp>

namespace voxer {

class SliceRenderer {
public:
  using MarkType = std::pair<Dataset *, std::array<float, 3>>;

  void set_dataset(Dataset *dataset);
  void add_mark(Dataset *dataset, const std::string &hex_color);
  void add_mark(Dataset *dataset, const std::array<float, 3> &color);
  [[nodiscard]] Image render(Dataset::Axis axis, uint32_t slice) const;

private:
  Dataset *m_dataset = nullptr;
  VolumeInfo m_info{};

  std::vector<MarkType> m_marks{};
};

} // namespace voxer