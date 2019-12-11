#pragma once
#include <memory>
#include <string>
#include <vector>
#include <voxer/Camera.hpp>
#include <voxer/Isosurface.hpp>
#include <voxer/Light.hpp>
#include <voxer/Slice.hpp>
#include <voxer/TransferFunction.hpp>
#include <voxer/Volume.hpp>

namespace voxer {

struct Scene {
  Camera camera;
  std::vector<const Dataset *> datasets;
  std::vector<TransferFunction> tfcns;
  std::vector<Volume> volumes;
  std::vector<Isosurface> isosurfaces;
  std::vector<Slice> slices;
  std::vector<Light> lights;
};

} // namespace voxer
