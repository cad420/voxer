#pragma once
#include "DataModel/Camera.hpp"
#include "DataModel/Dataset.hpp"
#include "DataModel/Isosurface.hpp"
#include "DataModel/TransferFunction.hpp"
#include "DataModel/Volume.hpp"
#include <seria/object.hpp>
#include <string>
#include <vector>

namespace voxer::remote {

struct Scene {
  std::vector<TransferFunction> tfcns;
  std::vector<Volume> volumes;
  std::vector<Isosurface> isosurfaces;
  Camera camera;
};

} // namespace voxer::remote

namespace seria {

template <> inline auto register_object<voxer::remote::Scene>() {
  using Scene = voxer::remote::Scene;
  return std::make_tuple(member("volumes", &Scene::volumes),
                         member("tfcns", &Scene::tfcns),
                         member("isosurfaces", &Scene::isosurfaces),
                         member("camera", &Scene::camera));
}

} // namespace seria
