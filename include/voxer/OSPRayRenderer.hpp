#include <voxer/Renderer.hpp>

namespace voxer {
class OSPRayRenderer : public Renderer {
protected:
  Image renderImage(const CameraConfig &cameraConfig,
                    const std::vector<VolumeConfig> &volumeConfigs,
                    const std::vector<SliceConfig> &sliceConfigs,
                    const std::vector<IsosurfaceConfig> &isosurfaceConfigs,
                    const std::vector<std::string> &volumesToRender,
                    const ospcommon::vec2i &size) override;
};
}
