#pragma once
#include <voxer/Data/StructuredGrid.hpp>
#include <voxer/Rendering/Isosurface.hpp>
#include <voxer/Rendering/Camera.hpp>
#include <voxer/Data/Image.hpp>

class VoxerIRenderer {
public:
  virtual void set_camera(const voxer::Ca)
  virtual void render() = 0;
  virtual auto get_colors() -> const voxer::Image & = 0;

  virtual ~VoxerIRenderer() = default;
};
