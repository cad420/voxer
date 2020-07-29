#pragma once
#include <voxer/DatasetStore.hpp>
#include <voxer/Image.hpp>
#include <voxer/Scene.hpp>

class VoxerIRenderer {
public:
  virtual void render(const voxer::Scene &scene,
                      voxer::DatasetStore &datasets) = 0;
  virtual auto get_colors() -> const voxer::Image & = 0;

  virtual ~VoxerIRenderer() = default;
};
