#pragma once
#include "DataModel/DatasetInfo.hpp"
#include <cstdint>
#include <seria/object.hpp>

namespace voxer::remote {

struct Isosurface {
  DatasetID dataset;
  std::string color = "#FF0000";
  float value = 0.0f;
  bool render = true;
};

} // namespace voxer::remote

namespace seria {

template <> inline auto register_object<voxer::remote::Isosurface>() {
  using Isosurface = voxer::remote::Isosurface;
  return std::make_tuple(member("value", &Isosurface::value),
                         member("dataset", &Isosurface::dataset),
                         member("color", &Isosurface::color),
                         member("render", &Isosurface::render));
}

} // namespace seria
