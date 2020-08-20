#pragma once
#include <array>
#include <seria/utils.hpp>

namespace voxer::remote {

struct Dataset {
  std::string name;
  std::string variable;
  uint32_t timestep = 0;

  bool operator==(const Dataset &other) const {
    return (name == other.name && variable == other.variable &&
            timestep == other.timestep);
  }
};

} // namespace voxer::remote

namespace seria {

template <> inline auto registerObject<voxer::remote::Dataset>() {
  using Dataset = voxer::remote::Dataset;
  return std::make_tuple(member("name", &Dataset::name),
                         member("variable", &Dataset::variable),
                         member("timestep", &Dataset::timestep));
}

} // namespace seria