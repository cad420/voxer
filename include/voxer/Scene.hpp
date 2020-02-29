#pragma once
#include <string>
#include <vector>
#include <voxer/scene/Camera.hpp>
#include <voxer/scene/Isosurface.hpp>
#include <voxer/scene/Light.hpp>
#include <voxer/scene/SceneDataset.hpp>
#include <voxer/scene/Slice.hpp>
#include <voxer/scene/TransferFunction.hpp>
#include <voxer/scene/Volume.hpp>

namespace voxer {

struct Scene {
  std::vector<SceneDataset> datasets;
  std::vector<TransferFunction> tfcns;
  std::vector<Volume> volumes;
  std::vector<Isosurface> isosurfaces;
  std::vector<Slice> slices;
  std::vector<Light> lights;
  Camera camera;

  auto serialize() -> rapidjson::Document;
  static auto deserialize(const rapidjson::Value &json) -> Scene;
};

} // namespace voxer
