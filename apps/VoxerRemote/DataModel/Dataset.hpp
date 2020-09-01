#pragma once
#include <array>
#include <seria/object.hpp>

namespace voxer::remote {

struct Dataset {
  uint32_t id;
  std::string name;
  std::string variable;
  uint32_t timestep = 0;
  std::string path;

  bool operator==(const Dataset &other) const {
    return id == other.id;
  }
};

} // namespace voxer::remote

namespace seria {

template <> inline auto register_object<voxer::remote::Dataset>() {
  using Dataset = voxer::remote::Dataset;
  return std::make_tuple(
      member("id", &Dataset::id), member("name", &Dataset::name),
      member("variable", &Dataset::variable),
      member("timestep", &Dataset::timestep), member("path", &Dataset::path));
}

} // namespace seria

namespace std {
template <> struct hash<voxer::remote::Dataset> {
  size_t operator()(const voxer::remote::Dataset &s) const {
    // Compute individual hash values for first, second and third
    // http://stackoverflow.com/a/1646913/126995
    size_t res = 17;
    res = res * 31 + hash<string>()(s.name);
    res = res * 31 + hash<string>()(s.variable);
    res = res * 31 + hash<uint32_t>()(s.timestep);
    return res;
  }
};
} // namespace std