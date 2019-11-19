#pragma once
#include <vector>
#include <string>
#include <voxer/configs/VolumeConfig.hpp>
#include <voxer/configs/CameraConfig.hpp>
#include <voxer/configs/SliceConfig.hpp>
#include <voxer/configs/SliceConfig.hpp>
#include <voxer/configs/IsosurfaceConfig.hpp>

namespace voxer {

struct Config {
  ospcommon::vec2i size;
  std::vector<VolumeConfig> volumeConfigs;
  CameraConfig cameraConfig;
  std::vector<SliceConfig> sliceConfigs;
  std::vector<IsosurfaceConfig> isosurfaceConfigs;
  std::vector<std::string> volumesToRender;
  // std::vector<LightConfig> lightConfigs;
  Config(){};
};
}
