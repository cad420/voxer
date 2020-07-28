#pragma once
#include "Rendering/IRenderingContext.hpp"
#include <ospray/ospray_util.h>

namespace voxer {

class RenderingContextOSPRay final : public VoxerIRenderingContext {
public:
  RenderingContextOSPRay();

  void render(const Scene &scene, DatasetStore &datasets) final;
  auto get_colors() -> const Image & final;

  auto render_slice(const Dataset &dataset) -> Image;

private:
  void create_osp_volume(const Dataset &dataset);
  OSPVolume &get_osp_volume(uint32_t idx,
                            const std::vector<SceneDataset> &scene_datasets,
                            DatasetStore &datasets);

  std::map<std::string, OSPVolume> osp_volume_cache{};
  Image image{};
};

} // namespace voxer

extern "C" {
VoxerIRenderingContext *voxer_get_backend() {
  return new voxer::RenderingContextOSPRay();
}
}