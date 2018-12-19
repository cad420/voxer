#pragma once
#include "ParallelRenderer/ConfigManager.h"
#include "ParallelRenderer/DatasetManager.h"
#include "Poco/URI.h"
#include "config/CameraConfig.h"
#include "ospray/ospray_cpp.h"
#include "third_party/rapidjson/document.h"
#include <map>
#include <string>
#include <vector>

typedef std::vector<unsigned char> Image;

class Renderer {
public:
  Image render(const Config &config);
  Image render(const Config &config, const ospcommon::vec2i &size,
               const CameraConfig &cameraConfig);
  // Image render(const Config &config, const ospcommon::vec2i &size,
  //              const CameraConfig &cameraConfig,
  //              std::map<std::string, TransferFunctionConfig> &tfcnConfigs);

private:
  Image renderImage(const CameraConfig &cameraConfig,
                    const std::vector<VolumeConfig> &volumeConfigs,
                    const std::vector<SliceConfig> &sliceConfigs,
                    const std::vector<IsosurfaceConfig> &isosurfaceConfigs,
                    const std::vector<std::string> &volumesToRender,
                    const ospcommon::vec2i &size);
};