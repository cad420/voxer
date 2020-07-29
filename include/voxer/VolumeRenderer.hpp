#pragma once
#include <voxer/DatasetStore.hpp>
#include <voxer/Image.hpp>
#include <voxer/Scene.hpp>

class VoxerIRenderer;

namespace voxer {

class VolumeRenderer {
public:
  enum struct Type { OSPRay, OpenGL };

  explicit VolumeRenderer(Type type);
  ~VolumeRenderer();

  void render(const Scene &scene, DatasetStore &datasets) const;

  auto get_colors() -> const Image &;

private:
  std::unique_ptr<VoxerIRenderer> impl;
};

} // namespace voxer