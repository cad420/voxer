#pragma once
#include <cstdint>
#include <vector>

namespace voxer {

inline auto conversion(const float *source, size_t size, float max, float min)
    -> std::vector<uint8_t> {
  std::vector<uint8_t> data;
  data.reserve(size);

  double max_range = max - min;
  uint8_t max_value = 255;
  for (size_t i = 0; i < size; i++) {
    double range = source[i] - min;
    data.push_back(range / max_range * 255);
  }

  return data;
}

inline auto conversion(const float *source, size_t size)
    -> std::vector<uint8_t> {
  auto min = source[0];
  auto max = source[0];
  for (size_t i = 0; i < size; i++) {
    auto value = source[i];
    if (value > max) {
      max = value;
    } else if (value < min) {
      min = value;
    }
  }
  return conversion(source, size, max, min);
}

} // namespace voxer