#pragma once
#include <string>
#include <vector>
#include <voxer/Scene/Camera.hpp>
#include <voxer/Scene/Isosurface.hpp>
#include <voxer/Scene/Light.hpp>
#include <voxer/Scene/SceneDataset.hpp>
#include <voxer/Scene/TransferFunction.hpp>
#include <voxer/Scene/Volume.hpp>

namespace voxer {

struct Scene {
  std::vector<SceneDataset> datasets;
  std::vector<TransferFunction> tfcns;
  std::vector<Volume> volumes;
  std::vector<Isosurface> isosurfaces;
  std::vector<Light> lights;
  Camera camera;

};

} // namespace voxer
