#pragma once
#include <voxer/DatasetStore.hpp>
#include <voxer/Image.hpp>
#include <voxer/Scene.hpp>

class VoxerIRenderingContext;

namespace voxer {

class RenderingContext {
public:
  enum struct Type { OSPRay, OpenGL };

  explicit RenderingContext(Type type);
  ~RenderingContext();

  void render(const Scene &scene, DatasetStore &datasets);
  auto get_colors() -> const Image &;

private:
  std::unique_ptr<VoxerIRenderingContext> impl;
};

} // namespace voxer