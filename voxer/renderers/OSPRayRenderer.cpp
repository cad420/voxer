#include "voxer/utils/Debugger.hpp"
#include "voxer/configs/CameraConfig.hpp"
#include "voxer/configs/TransferFunctionConfig.hpp"
#include "voxer/managers/UserManager.hpp"
#include "voxer/renderers/Renderer.hpp"
#include <algorithm>
#include <cstdlib>

using namespace std;
using namespace ospcommon;
namespace o = ospray::cpp;

extern DatasetManager datasets;
extern UserManager users;

static Debugger debug("renderer");

Image OSPRayRenderer::renderImage(
    const CameraConfig &cameraConfig, const vector<VolumeConfig> &volumeConfigs,
    const vector<SliceConfig> &sliceConfigs,
    const vector<IsosurfaceConfig> &isosurfaceConfigs,
    const vector<string> &volumesToRender, const vec2i &size) {
  auto start = chrono::steady_clock::now();

  o::Model world;

  vector<o::Volume> volumes;
  vector<string> volumeIds;
  for (auto &volumeConfig : volumeConfigs) {
    const auto &tfcnConfig = volumeConfig.tfcnConfig;
    vector<float> opacities(255, 0);
    if (volumeConfig.ranges.size() != 0) {
      for (auto range : volumeConfig.ranges) {
        for (auto i = range.start; i < range.end && i <= 255; i++) {
          opacities[i] = tfcnConfig.opacities[i];
        }
      }
    } else {
      opacities = tfcnConfig.opacities;
    }
    o::Data colorsData(tfcnConfig.colors.size(), OSP_FLOAT3,
                       tfcnConfig.colors.data());
    o::Data opacityData(opacities.size(), OSP_FLOAT, opacities.data());
    colorsData.commit();
    opacityData.commit();
    vec2f valueRange{0, 255};

    o::TransferFunction tfcn("piecewise_linear");
    tfcn.set("colors", colorsData);
    tfcn.set("opacities", opacityData);
    tfcn.set("valueRange", valueRange);
    tfcn.commit();

    auto &datasetConfig = volumeConfig.datasetConfig;
    auto &dataset = datasets.get(datasetConfig.name);

    auto &user = users.get("tester");
    auto &volume = user.get(datasetConfig.name);

    volume.set("transferFunction", tfcn);
    volume.commit();
    volumes.push_back(volume);
    volumeIds.push_back(volumeConfig.id);
  }

  o::Model model;
  for (auto id : volumesToRender) {
    auto pos = find(volumeIds.begin(), volumeIds.end(), id);
    auto volume = volumes[pos - volumeIds.begin()];
    auto &config = volumeConfigs[pos - volumeIds.begin()];
    auto &datasetConfig = config.datasetConfig;

    // https://github.com/ospray/ospray/issues/159#issuecomment-444155750
    volume.set("gridOrigin",
               vec3f(-datasetConfig.dimensions / 2) + config.translate);
    volume.set("gridSpacing", config.gridSpacing);
    volume.set("voxelRange", vec2f(0, 255));
    // volume.set("gradientShadingEnabled", true);
    volume.set("singleShade", false);

    // https://github.com/ospray/ospray/pull/165
    // https://github.com/ospray/ospray/issues/159#issuecomment-443847715
    // volume.set("xfm.l.vx", vec3f{0.01, 0.0, 0.0});
    // volume.set("xfm.l.vy", vec3f{0.0, 1.0, 0.0});
    // volume.set("xfm.l.vz", vec3f{0.0, 0.0, 1.0});
    // volume.set("xfm.p", vec3f{0.0, 0.0, 0.0});
    // volume.commit();

    volume.set("volumeClippingBoxLower", vec3f(datasetConfig.clipingBoxLower));
    volume.set("volumeClippingBoxUpper", vec3f(datasetConfig.clipingBoxUpper));

    world.addVolume(volume);

    world.commit();
  }

  if (sliceConfigs.size() > 0) {
    vector<string> ids;
    vector<vector<vec4f>> planesForAll;
    for (auto &sliceConfig : sliceConfigs) {
      auto pos = find(ids.begin(), ids.end(), sliceConfig.volumeId);
      vec4f coeff = {sliceConfig.a, sliceConfig.b, sliceConfig.c,
                     sliceConfig.d};
      if (pos == ids.end()) {
        vector<vec4f> planes = {coeff};
        planesForAll.push_back(planes);
        ids.push_back(sliceConfig.volumeId);
      } else {
        auto planes = planesForAll[pos - ids.begin()];
        planes.push_back(coeff);
      }
    }
    for (auto i = 0; i < planesForAll.size(); i++) {
      o::Geometry slice("slices");
      o::Data planesData(planesForAll[i].size(), OSP_FLOAT4,
                         planesForAll[i].data());
      auto pos = find(volumeIds.begin(), volumeIds.end(), ids[i]);
      slice.set("planes", planesData);
      slice.set("volume", volumes[i]);
      world.addGeometry(slice);
    }
  }

  if (isosurfaceConfigs.size() > 0) {
    vector<string> ids;
    vector<vector<float>> valuesForAll;
    for (auto &isosurfaceConfig : isosurfaceConfigs) {
      auto pos = find(ids.begin(), ids.end(), isosurfaceConfig.volumeId);
      auto value = isosurfaceConfig.value;
      if (pos == ids.end()) {
        vector<float> values = {value};
        valuesForAll.push_back(values);
        ids.push_back(isosurfaceConfig.volumeId);
      } else {
        auto values = valuesForAll[pos - ids.begin()];
        values.push_back(value);
      }
    }
    for (auto i = 0; i < valuesForAll.size(); i++) {
      o::Geometry isosurface("isosurfaces");
      o::Data valuesData(valuesForAll[i].size(), OSP_FLOAT,
                         valuesForAll[i].data());
      auto pos = find(volumeIds.begin(), volumeIds.end(), ids[i]);
      isosurface.set("isovalues", valuesData);
      isosurface.set("volume", volumes[pos - volumeIds.begin()]);
      isosurface.commit();
      world.addGeometry(isosurface);
    }
  }

  world.commit();

  vector<OSPLight> lights;
  o::Light light("ambient");
  light.set("intensity", 1.25);
  light.set("color", vec3f(1, 1, 1));
  light.commit();
  lights.push_back(light.handle());

  o::Camera camera(cameraConfig.type);
  camera.set("aspect", size.x / (float)size.y);
  camera.set("pos", cameraConfig.pos);
  camera.set("dir", cameraConfig.dir);
  camera.set("up", cameraConfig.up);
  camera.commit();

  // create renderer
  o::Renderer renderer("scivis");
  renderer.set("lights", o::Data(lights.size(), OSP_LIGHT, lights.data()));
  // renderer.set("aoSamples", 1);
  renderer.set("spp", size.x == 64 ? 1 : 2);
  renderer.set("aoDistance", 10000.f);
  renderer.set("bgColor", 1.0f);
  renderer.set("camera", camera);
  renderer.set("model", world);
  renderer.commit();

  // render frame
  const size_t iterationTimes = size.x == 64 ? 5 : 10;
  o::FrameBuffer framebuffer(size, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
  framebuffer.clear(OSP_FB_COLOR | OSP_FB_ACCUM);
  for (int frames = 0; frames < iterationTimes; frames++) {
    renderer.renderFrame(framebuffer, OSP_FB_COLOR | OSP_FB_ACCUM);
  }

  // get frame data
  // TODO: too ugly
  unsigned char *fb = (unsigned char *)framebuffer.map(OSP_FB_COLOR);
  vector<unsigned char> image(fb, fb + size.x * size.y * 4);
  framebuffer.unmap(fb);

  renderer.release();
  camera.release();
  light.release();
  framebuffer.release();
  world.release();

  const auto delta = chrono::duration_cast<chrono::milliseconds>(
      chrono::steady_clock::now() - start);
  debug.log(to_string(delta.count()) + " ms");

  return image;
};