#include "Renderer.h"
#include "config/CameraConfig.h"
#include "config/TransferFunctionConfig.h"
#include <cstdlib>

using namespace std;
using namespace ospcommon;
namespace o = ospray::cpp;

extern DatasetManager datasets;

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
    gensv::LoadedVolume volume;

    const auto &tfcnConfig = volumeConfig.tfcnConfig;
    ospray::cpp::Data colorsData(tfcnConfig.colors.size(), OSP_FLOAT3,
                                 tfcnConfig.colors.data());
    ospray::cpp::Data opacityData(tfcnConfig.opacities.size(), OSP_FLOAT,
                                  tfcnConfig.opacities.data());
    colorsData.commit();
    opacityData.commit();
    vec2f valueRange{0, 255};
    if (volumeConfig.name == "magnetic") {
      valueRange.x = 0.44;
      valueRange.y = 0.77;
    }

    volume.tfcn.set("colors", colorsData);
    volume.tfcn.set("opacities", opacityData);
    volume.tfcn.set("valueRange", valueRange);
    volume.tfcn.commit();

    auto &datasetConfig = volumeConfig.datasetConfig;
    auto &dataset = datasets.get(datasetConfig.name);
    gensv::loadVolume(volume, dataset.buffer, dataset.dimensions, dataset.dtype,
                      dataset.sizeForDType);
    const auto halfLength = dataset.dimensions / 2;
    volume.bounds.lower -= ospcommon::vec3f(halfLength);
    volume.bounds.upper -= ospcommon::vec3f(halfLength);
    volume.volume.set("gridOrigin",
                      volume.ghostGridOrigin - ospcommon::vec3f(halfLength));
    volume.volume.set("transferFunction", volume.tfcn);
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

  cout << "render: "
       << chrono::duration_cast<chrono::milliseconds>(
              chrono::steady_clock::now() - start)
              .count()
       << " ms " << endl;

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
