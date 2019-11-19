#include <voxer/Renderer.hpp>
#include <voxer/configs/TransferFunctionConfig.hpp>
#include <voxer/UserManager.hpp>
#include "utils/Debugger.hpp"
#include <algorithm>
#include <cstdlib>

using namespace std;
using namespace ospcommon;

namespace voxer {

Image Renderer::render(const Config &config) {
  return this->renderImage(config.cameraConfig, config.volumeConfigs,
                           config.sliceConfigs, config.isosurfaceConfigs,
                           config.volumesToRender, config.size);
}

// for http get
Image Renderer::render(const Config &config, const vec2i &size,
                       const CameraConfig &cameraConfig) {
  return this->renderImage(cameraConfig, config.volumeConfigs,
                           config.sliceConfigs, config.isosurfaceConfigs,
                           config.volumesToRender, size);
}

}
