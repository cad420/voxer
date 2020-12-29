#pragma once
#include <unordered_map>
#include <voxer/Data/Camera.hpp>
#include <voxer/Data/Image.hpp>
#include <voxer/Data/Isosurface.hpp>
#include <voxer/Data/Volume.hpp>

class VoxerIRenderer;

namespace voxer {

using GetRenderingBackend = VoxerIRenderer *(*)();

class VolumeRenderer {
public:
  explicit VolumeRenderer(const char *renderer);
  ~VolumeRenderer();

  auto get_name() const noexcept -> const char *;
  void set_background(float r, float g, float b) noexcept;
  void set_camera(const Camera &) noexcept;
  void add_volume(const std::shared_ptr<Volume> &volume);
  void add_isosurface(const std::shared_ptr<Isosurface> &isosurface);
  void clear_scene();
  bool has_cache(StructuredGrid *data) const noexcept;

  void render() const;

  auto get_colors() -> const Image &;

private:
  std::string m_name;
  std::unique_ptr<VoxerIRenderer> m_impl;
  static std::unordered_map<std::string, GetRenderingBackend> symbols;
};

} // namespace voxer