#pragma once
#include <array>
#include <cstdint>
#include <seria/utils.hpp>
#include <string>
#include <vector>

namespace voxer::remote {

struct DatasetCollection {
  struct Variable {
    std::string name;
    uint32_t timesteps = 0;
    std::string path;
  };
  std::string name;
  std::string type;
  std::array<uint32_t, 3> dimensions{0, 0, 0};
  std::vector<Variable> variables;
};

} // namespace voxer::remote

namespace seria {

template <>
inline auto registerObject<voxer::remote::DatasetCollection::Variable>() {
  using Variable = voxer::remote::DatasetCollection::Variable;
  return std::make_tuple(member("name", &Variable::name),
                         member("timesteps", &Variable::timesteps),
                         member("path", &Variable::path));
}

template <> inline auto registerObject<voxer::remote::DatasetCollection>() {
  using DatasetCollection = voxer::remote::DatasetCollection;
  return std::make_tuple(member("name", &DatasetCollection::name),
                         member("type", &DatasetCollection::type),
                         member("dimensions", &DatasetCollection::dimensions),
                         member("variables", &DatasetCollection::variables));
}

} // namespace seria