#pragma once
#include <voxer/DatasetStore.hpp>
#include <voxer/Image.hpp>
#include <voxer/Scene.hpp>

namespace voxer {

class IRenderingContext {
public:
  virtual void render(const Scene &scene, DatasetStore &datasets) = 0;
  virtual auto get_colors() -> const Image & = 0;

  virtual ~IRenderingContext() = default;
};

} // namespace voxer
