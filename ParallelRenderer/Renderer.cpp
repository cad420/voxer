#include "Renderer.h"
#include "config/CameraConfig.h"
#include "ParallelRenderer/UserManager.h"
#include "config/TransferFunctionConfig.h"
#include "util/Debugger.h"
#include <cstdlib>

using namespace std;
using namespace ospcommon;
namespace o = ospray::cpp;

extern DatasetManager datasets;
extern UserManager users;
static Debugger debug("renderer");

Image Renderer::renderImage(const CameraConfig &cameraConfig,
                            const vector<VolumeConfig> &volumeConfigs,
                            const vec2i &size) {
  auto start = chrono::steady_clock::now();

  o::Camera camera("perspective");
  camera.set("aspect", size.x / (float)size.y);
  camera.set("pos", cameraConfig.pos);
  camera.set("dir", cameraConfig.dir);
  camera.set("up", cameraConfig.up);
  camera.commit();

  o::Model model;
  for (auto volumeConfig : volumeConfigs) {
    const auto &tfcnConfig = volumeConfig.tfcnConfig;
    o::Data colorsData(tfcnConfig.colors.size(), OSP_FLOAT3,
                                 tfcnConfig.colors.data());
    o::Data opacityData(tfcnConfig.opacities.size(), OSP_FLOAT,
                                  tfcnConfig.opacities.data());
    colorsData.commit();
    opacityData.commit();
    vec2f valueRange{0, 255};
    if (volumeConfig.name == "magnetic") {
      valueRange.x = 0.44;
      valueRange.y = 0.77;
    }

    o::TransferFunction tfcn("piecewise_linear");
    tfcn.set("colors", colorsData);
    tfcn.set("opacities", opacityData);
    tfcn.set("valueRange", valueRange);
    tfcn.commit();
 
    auto &datasetConfig = volumeConfig.datasetConfig;
    auto &dataset = datasets.get(datasetConfig.name);

    auto &user = users.get("tester");
    auto &volume = user.get(datasetConfig.name);
    volume.volume.set("transferFunction", tfcn);
    volume.volume.commit();

    vector<box3f> regions{volume.bounds};
    o::Data regionData(regions.size() * 2, OSP_FLOAT3, regions.data());
    model.addVolume(volume.volume);
    model.set("regions", regionData);
  }

  model.commit();

  vector<OSPLight> lights;
  o::Light light("scivis", "ambient");
  light.commit();
  lights.push_back(light.handle());

  // create renderer
  o::Renderer renderer("scivis");
  renderer.set("lights", o::Data(lights.size(), OSP_LIGHT, lights.data()));
  renderer.set("aoSamples", 0);
  renderer.set("bgColor", 1.0f);
  renderer.set("camera", camera);
  renderer.set("model", model);
  renderer.commit();

  // render frame
  const size_t iterationTimes = 5;
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
  model.release();

  const auto delta = chrono::duration_cast<chrono::milliseconds>(
      chrono::steady_clock::now() - start);
  debug.log(to_string(delta.count()) + " ms");

  return image;
};

Image Renderer::render(const Config &config) {
  return this->renderImage(config.cameraConfig, config.volumeConfigs,
                           config.size);
}

// for http get
Image Renderer::render(const Config &config, const vec2i &size,
                       const CameraConfig &cameraConfig) {
  return this->renderImage(cameraConfig, config.volumeConfigs, size);
}
