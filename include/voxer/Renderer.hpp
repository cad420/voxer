#pragma once
#include <voxer/Config.hpp>
#include "voxer/configs/CameraConfig.hpp"
#include "third_party/rapidjson/document.h"
#include "voxer/managers/ConfigManager.hpp"
#include "voxer/managers/DatasetManager.hpp"
#include <map>
#include <ospray/ospray_cpp.h>
#include <string>
#include <vector>

namespace voxer {

using Image = std::vector<unsigned char>;

class Renderer {
public:
  Image render(const Config &config);
  Image render(const Config &config, const ospcommon::vec2i &size,
               const CameraConfig &cameraConfig);
  // Image render(const Config &config, const ospcommon::vec2i &size,
  //              const CameraConfig &cameraConfig,
  //              std::map<std::string, TransferFunctionConfig> &tfcnConfigs);

protected:
  virtual Image
  renderImage(const CameraConfig &cameraConfig,
              const std::vector<VolumeConfig> &volumeConfigs,
              const std::vector<SliceConfig> &sliceConfigs,
              const std::vector<IsosurfaceConfig> &isosurfaceConfigs,
              const std::vector<std::string> &volumesToRender,
              const ospcommon::vec2i &size) = 0;
};
}
