#pragma once
#include "Camera.hpp"
#include "Isosurface.hpp"
#include "SceneDataset.hpp"
#include "Slice.hpp"
#include "TransferFunction.hpp"
#include "Volume.hpp"
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>
#include <voxer/Scene.hpp>

namespace seria {

template <> inline auto registerObject<voxer::Scene>() {
  using Scene = voxer::Scene;
  // TODO: parse lights
  return std::make_tuple(member("datasets", &Scene::datasets),
                         member("volumes", &Scene::volumes),
                         member("tfcns", &Scene::tfcns),
                         member("isosurfaces", &Scene::isosurfaces),
                         member("camera", &Scene::camera));
}

} // namespace seria
