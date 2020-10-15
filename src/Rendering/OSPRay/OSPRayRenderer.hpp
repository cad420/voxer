#pragma once
#include "Rendering/IRenderer.hpp"
#include <unordered_map>
#include <ospray/ospray.h>
#include <ospray/ospray_util.h>

namespace voxer {

class OSPRayRenderer final : public VoxerIRenderer {
public:
  OSPRayRenderer();
  ~OSPRayRenderer() final;

  void set_camera(const Camera &) noexcept override;
  void set_background(float r, float g, float b) noexcept override;
  void add_volume(const std::shared_ptr<Volume> &) noexcept override;
  void add_isosurface(const std::shared_ptr<voxer::Isosurface> &) noexcept override;
  void render() final;
  auto get_colors() -> const Image & final;
  void clear_scene() noexcept override;

private:
  OSPVolume& create_osp_volume(StructuredGrid *volume);
  OSPVolume &get_osp_volume(StructuredGrid *volume);

  std::unordered_map<StructuredGrid *, OSPVolume> m_osp_volume_cache{};
  std::unordered_map<Volume *, OSPVolumetricModel> m_osp_volume_models_cache{};
  std::unordered_map<Isosurface *, OSPGeometricModel> m_osp_isosurface_models_cache{};
  Image m_image{};
  std::array<float, 3> m_background {};
  std::vector<std::shared_ptr<Volume>> m_volumes;
  std::vector<std::shared_ptr<Isosurface>> m_isosurfaces;
  Camera m_camera;

  OSPDevice osp_device = nullptr;
};

} // namespace voxer

extern "C" {
inline VoxerIRenderer *voxer_get_backend() { return new voxer::OSPRayRenderer(); }
}