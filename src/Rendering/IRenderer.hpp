#pragma once
#include <voxer/Data/Image.hpp>
#include <voxer/Data/StructuredGrid.hpp>
#include <voxer/Rendering/Camera.hpp>
#include <voxer/Rendering/Isosurface.hpp>
#include <voxer/Rendering/Volume.hpp>

class VoxerIRenderer {
public:
  virtual void set_camera(const voxer::Camera &) = 0;
  virtual void set_background(float r, float g, float b) noexcept = 0;
  virtual void add_volume(const std::shared_ptr<voxer::Volume> &) = 0;
  virtual void add_isosurface(const std::shared_ptr<voxer::Isosurface> &) = 0;
  virtual void render() = 0;
  virtual auto get_colors() -> const voxer::Image & = 0;
  virtual void clear_scene() = 0;
  virtual bool has_cache(voxer::StructuredGrid *data) const noexcept = 0;

  virtual ~VoxerIRenderer() = default;
};
