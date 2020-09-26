#pragma once
#include <array>
#include <vector>
#include <voxer/Data/Color.hpp>

namespace voxer {

struct ControlPoint {
  float x = 0;
  float y = 0;
  std::array<float, 3> color = {0, 0, 0};
};

class TransferFunction {
public:
  [[nodiscard]] auto get_points() const -> const std::vector<ControlPoint> & {
    return points;
  }

  void add_point(float x, float y, std::array<float, 3> color);

  auto interpolate()
      -> std::pair<std::vector<float>, std::vector<std::array<float, 3>>>;

private:
  std::vector<ControlPoint> points;
  std::vector<float> opacities;
  std::vector<std::array<float, 3>> colors;
  bool is_valid = false;
};

} // namespace voxer
