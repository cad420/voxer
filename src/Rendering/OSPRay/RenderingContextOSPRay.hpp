#pragma once
#include "Rendering/IRenderingContext.hpp"
#include <ospray/ospray.h>

namespace voxer {

struct OSPRayRendererCache {
  void set_datasets(const std::vector<Dataset> &datasets) {
    // TODO: OSPVolume creation is really expensive
    // TODO: OSPVolume modification is not thread safe!!!
    for (auto &dataset : datasets) {
      this->create_osp_volume(dataset);
    }
  }

  void create_osp_volume(const Dataset &dataset) {
    auto &info = dataset.info;
    auto &dimensions = info.dimensions;
    auto osp_volume = ospNewVolume("block_bricked_volume");
    ospSet3i(osp_volume, "dimensions", dimensions[0], dimensions[1],
             dimensions[2]);
    ospSetString(osp_volume, "voxelType", "uchar");
    ospSetRegion(
        osp_volume,
        reinterpret_cast<void *>(const_cast<uint8_t *>(dataset.buffer.data())),
        osp::vec3i{0, 0, 0},
        osp::vec3i{static_cast<int>(dimensions[0]),
                   static_cast<int>(dimensions[1]),
                   static_cast<int>(dimensions[2])});
    ospCommit(osp_volume);
    osp_volumes.emplace(dataset.id, osp_volume);
  }
  // TODO: should be dataset cache
  std::map<std::string, OSPVolume> osp_volumes;
};

class RenderingContextOSPRay : public IRenderingContext {
public:
  RenderingContextOSPRay();

  void render(const Scene &scene, DatasetStore &datasets) final;
  auto get_colors() -> const Image & final;

private:
  std::unique_ptr<OSPRayRendererCache> cache;
  Image image{};
};

} // namespace voxer
