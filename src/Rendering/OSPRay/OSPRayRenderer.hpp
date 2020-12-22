#pragma once
#include "Rendering/IRenderer.hpp"
#include "Rendering/OSPRay/OSPRayVolumeCache.hpp"
#include <ospray/ospray.h>
#include <ospray/ospray_util.h>
#include <unordered_map>

namespace voxer {

class OSPRayRenderer final : public VoxerIRenderer {
public:
  OSPRayRenderer();
  ~OSPRayRenderer() final;

  void set_camera(const Camera &) noexcept override;
  void set_background(float r, float g, float b) noexcept override;
  void add_volume(const std::shared_ptr<Volume> &) noexcept override;
  void
  add_isosurface(const std::shared_ptr<voxer::Isosurface> &) noexcept override;
  void render() final;
  auto get_colors() -> const Image & final;
  void clear_scene() noexcept override;
  bool has_cache(voxer::StructuredGrid *data) const noexcept override;

private:
  OSPRayVolumeCache *m_cache;
  Image m_image{};
  std::array<float, 3> m_background{};
  std::vector<std::shared_ptr<Volume>> m_volumes;
  std::vector<std::shared_ptr<Isosurface>> m_isosurfaces;
  Camera m_camera;
};

} // namespace voxer

extern "C" {
VoxerIRenderer *voxer_get_backend();
}