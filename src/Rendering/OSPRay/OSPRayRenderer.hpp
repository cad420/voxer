#pragma once
#include "Rendering/IRenderer.hpp"
#include <ospray/ospray.h>
#include <ospray/ospray_util.h>

namespace voxer {

class OSPRayRenderer final : public VoxerIRenderer {
public:
  OSPRayRenderer();
  ~OSPRayRenderer() final;

  void set_camera(const Camera &) override;
  void add_volume(const std::shared_ptr<Volume> &) override;
  void add_isosurface(const std::shared_ptr<voxer::Isosurface> &) override;
  void render() final;
  auto get_colors() -> const Image & final;
  void clear_scene() override;

private:
  OSPVolume& create_osp_volume(StructuredGrid *volume);
  OSPVolume &get_osp_volume(StructuredGrid *volume);

  std::map<StructuredGrid *, OSPVolume> m_osp_volume_cache{};
  Image m_image{};
  std::vector<std::shared_ptr<Volume>> m_volumes;
  std::vector<std::shared_ptr<Isosurface>> m_isosurfaces;
  Camera m_camera;

  OSPDevice osp_device = nullptr;
};

} // namespace voxer

extern "C" {
inline VoxerIRenderer *voxer_get_backend() { return new voxer::OSPRayRenderer(); }
}