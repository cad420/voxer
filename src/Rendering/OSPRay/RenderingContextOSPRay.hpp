#pragma once
#include "Rendering/IRenderingContext.hpp"
#include <ospray/ospray_util.h>

namespace voxer {

class RenderingContextOSPRay : public IRenderingContext {
public:
  RenderingContextOSPRay();

  void render(const Scene &scene, DatasetStore &datasets) final;
  auto get_colors() -> const Image & final;

private:
  void create_osp_volume(const Dataset &dataset);
  OSPVolume &get_osp_volume(uint32_t idx,
                            const std::vector<SceneDataset> &scene_datasets,
                            DatasetStore &datasets);

  std::map<std::string, OSPVolume> osp_volume_cache{};
  Image image{};
};

} // namespace voxer
