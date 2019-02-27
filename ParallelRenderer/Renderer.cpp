#include "Renderer.h"
#include "ParallelRenderer/UserManager.h"
#include "config/CameraConfig.h"
#include "config/TransferFunctionConfig.h"
#include "util/Debugger.h"
#include <algorithm>
#include <cstdlib>

using namespace std;
using namespace ospcommon;
namespace o = ospray::cpp;

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
