#pragma once
#include <memory>
#include <vector>
#include <voxer/Camera.hpp>
#include <voxer/Isosurface.hpp>
#include <voxer/Slice.hpp>
#include <voxer/Volume.hpp>

namespace voxer {

struct Scene {
  Camera camera;
  std::vector<std::shared_ptr<Volume>> volumes;
  std::vector<std::shared_ptr<Isosurface>> isosurfaces;
  std::vector<std::shared_ptr<Slice>> slices;
};

} // namespace voxer
