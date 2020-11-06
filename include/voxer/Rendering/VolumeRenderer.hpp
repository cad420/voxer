#pragma once
#include <unordered_map>
#include <voxer/Data/Image.hpp>
#include <voxer/Rendering/Camera.hpp>
#include <voxer/Rendering/Isosurface.hpp>
#include <voxer/Rendering/Volume.hpp>

class VoxerIRenderer;

namespace voxer {

using GetRenderingBackend = VoxerIRenderer *(*)();

class VolumeRenderer {
public:
  explicit VolumeRenderer(const char *backend);
  ~VolumeRenderer();

  auto get_backend() const noexcept -> const char*;
  void set_background(float r, float g, float b) noexcept;
  void set_camera(const Camera &) noexcept;
  void add_volume(const std::shared_ptr<Volume> &volume);
  void add_isosurface(const std::shared_ptr<Isosurface> &isosurface);
  void clear_scene();

  void render() const;

  auto get_colors() -> const Image &;

private:
  std::string m_backend;
  std::unique_ptr<VoxerIRenderer> impl;
  std::unordered_map<std::string, GetRenderingBackend> symbols;
};

} // namespace voxer