#pragma once
#include <string>
#include <vector>
#include <voxer/Camera.hpp>
#include <voxer/Isosurface.hpp>
#include <voxer/Light.hpp>
#include <voxer/SceneDataset.hpp>
#include <voxer/Slice.hpp>
#include <voxer/TransferFunction.hpp>
#include <voxer/Volume.hpp>

namespace voxer {

struct Scene {
  std::vector<SceneDataset> datasets;
  std::vector<TransferFunction> tfcns;
  std::vector<Volume> volumes;
  std::vector<Isosurface> isosurfaces;
  std::vector<Slice> slices;
  std::vector<Light> lights;
  Camera camera;

  auto serialize() -> std::string;
  static auto deserialize(simdjson::ParsedJson::Iterator &pjh) -> Scene;
};

} // namespace voxer
