#include "utils/Debugger.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
// #include <ospray/ospray.h>
#include <ospray/ospray_cpp.h>
#include <voxer/OSPRayRenderer.hpp>

using namespace std;

namespace voxer {

static Debugger debug("renderer");

class OSPRayRenderer::Impl {
public:
  Impl();
  Image render(const Scene &scene);
};

OSPRayRenderer::OSPRayRenderer() {
  this->impl = make_unique<OSPRayRenderer::Impl>();
}

Image OSPRayRenderer::render(const Scene &scene) {
  return this->impl->render(scene);
};

OSPRayRenderer::Impl::Impl() {
  // initialize ospray
}

Image OSPRayRenderer::Impl::render(const Scene &scene) {
  auto start = chrono::steady_clock::now();

  OSPModel model;

  vector<OSPVolume> ospVolumes;
  vector<uint8_t> renderIdxs;
  for (auto i = 0u; i < scene.volumes.size(); i++) {
    const auto &volume = *(scene.volumes[i]);
    const auto &tfcn = *volume.tfcn;
    vector<float> opacities(255, 0);
    // if (volume.ranges.size() != 0) {
    //   for (auto range : volume.ranges) {
    //     for (auto i = range.start; i < range.end && i <= 255; i++) {
    //       opacities[i] = tfcn.opacities[i];
    //     }
    //   }
    // } else {
    //   opacities = tfcn.opacities;
    // }

    auto ospColorsData =
        ospNewData(tfcn.colors.size(), OSP_FLOAT3,
                   reinterpret_cast<const void *>(tfcn.colors.data()),
                   OSP_DATA_SHARED_BUFFER);
    auto ospOpacityData =
        ospNewData(tfcn.opacities.size(), OSP_FLOAT, opacities.data());

    array<uint8_t, 2> valueRange{0, 255};

    auto ospTfcn = ospNewTransferFunction("piecewise_linear");
    ospSetData(ospTfcn, "colors", ospColorsData);
    ospSetData(ospTfcn, "opacities", ospOpacityData);
    ospSet2f(ospTfcn, "valueRange", 0.0f, 255.0f);
    ospCommit(ospTfcn);

    const auto &dataset = *volume.dataset;
    auto ospVolume = ospNewVolume("shared_structured_volume");
    ospSetObject(ospVolume, "transferFunction", ospTfcn);

    // Data data(dataset.buffer.size(), OSP_UCHAR, dataset.buffer.data(),
    // OSP_DATA_SHARED_BUFFER);
    auto ospDataset =
        ospNewData(dataset.buffer.size(), OSP_UCHAR,
                   static_cast<const void *>(dataset.buffer.data()),
                   OSP_DATA_SHARED_BUFFER);
    ospSetData(ospVolume, "voxelData", ospDataset);
    ospCommit(ospVolume);

    ospVolumes.push_back(move(ospVolume));
    if (volume.render) {
      renderIdxs.push_back(i);
    }
  }

  auto ospModel = ospNewModel();

  for (auto idx : renderIdxs) {
    const auto &volume = *(scene.volumes[idx]);
    const auto &dataset = *volume.dataset;
    const auto &ospVolume = ospVolumes[idx];

    // https://github.com/ospray/ospray/issues/159#issuecomment-444155750
    ospSet3f(ospVolume, "gridOrigin", dataset.dimensions[0] / 2,
             dataset.dimensions[1] / 2, dataset.dimensions[2] / 2);
    ospSet2f(ospVolume, "voxelRange", 0.0f, 255.0f);
    ospSet1b(ospVolume, "singleShade", 0);
    ospSet1b(ospVolume, "gradientShadingEnabled", 1);

    // https://github.com/ospray/ospray/pull/165
    // https://github.com/ospray/ospray/issues/159#issuecomment-443847715
    // volume.set("xfm.l.vx", vec3f{0.01, 0.0, 0.0});
    // volume.set("xfm.l.vy", vec3f{0.0, 1.0, 0.0});
    // volume.set("xfm.l.vz", vec3f{0.0, 0.0, 1.0});
    // volume.set("xfm.p", vec3f{0.0, 0.0, 0.0});
    // volume.commit();

    ospSet3f(ospVolume, "volumeClippingBoxLower", volume.clipBoxLower[0],
             volume.clipBoxLower[1], volume.clipBoxLower[2]);
    ospSet3f(ospVolume, "volumeClippingBoxUpper", volume.clipBoxUpper[0],
             volume.clipBoxUpper[1], volume.clipBoxUpper[2]);
    ospCommit(ospVolume);

    ospAddVolume(ospModel, ospVolume);
  }

  for (auto i = 0ull; i < scene.isosurfaces.size(); i++) {
    auto &slice = *scene.slices[i];
    auto oPlanesData =
        ospNewData(1, OSP_FLOAT4, static_cast<void *>(slice.coef.data()));

    auto oSlice = ospNewGeometry("slices");
    ospSetData(oSlice, "planes", oPlanesData);
    ospSetObject(oSlice, "volume", ospVolumes[0]);
    ospCommit(oSlice);

    ospAddGeometry(ospModel, oSlice);
  }

  for (auto i = 0u; i < scene.isosurfaces.size(); i++) {
    auto &isosurface = *scene.isosurfaces[i];
    auto oIsovalueData =
        ospNewData(1, OSP_FLOAT, static_cast<void *>(&isosurface.value));

    auto oIsosurface = ospNewGeometry("isosurfaces");
    ospSetData(oIsosurface, "isovalues", oIsovalueData);
    ospSetObject(oIsosurface, "volume", ospVolumes[0]);
    ospCommit(oIsosurface);

    ospAddGeometry(ospModel, oIsosurface);
  }

  ospCommit(ospModel);

  vector<OSPLight> lights;
  auto oLight = ospNewLight3("ambient");
  ospSet1f(oLight, "intensity", 1.25f);
  ospCommit(oLight);

  lights.push_back(oLight);

  auto &camera = scene.camera;
  auto osp_camera = ospNewCamera("perspective");
  ospSet1f(osp_camera, "aspect",
           camera.width / static_cast<float>(camera.height));
  ospSet3f(osp_camera, "pos", camera.pos[0], camera.pos[1], camera.pos[2]);
  ospSet3f(osp_camera, "up", camera.up[0], camera.up[1], camera.up[2]);
  ospSet3f(osp_camera, "dir", camera.dir[0], camera.dir[1], camera.dir[2]);
  ospCommit(osp_camera);

  // create renderer
  auto osp_renderer = ospNewRenderer("scivis");
  ospSetData(osp_renderer, "lights",
             ospNewData(lights.size(), OSP_LIGHT, lights.data()));
  ospSet1i(osp_renderer, "spp", camera.width == 64 ? 1 : 2);
  // renderer.set("aoSamples", 1);
  ospSet1f(osp_renderer, "aoDistance", 1000.f);
  ospSet1f(osp_renderer, "bgColor", 1.0f);
  ospSetObject(osp_renderer, "camera", osp_camera);
  ospSetObject(osp_renderer, "model", model);
  ospCommit(osp_renderer);

  // render frame
  const size_t iterationTimes = camera.width == 64 ? 5 : 10;
  auto osp_framebuffer =
      ospNewFrameBuffer(osp::vec2i{camera.width, camera.height}, OSP_FB_SRGBA,
                        OSP_FB_COLOR | OSP_FB_ACCUM);
  ospFrameBufferClear(osp_framebuffer, OSP_FB_COLOR | OSP_FB_ACCUM);
  for (int frames = 0; frames < iterationTimes; frames++) {
    ospRenderFrame(osp_framebuffer, osp_renderer, OSP_FB_COLOR | OSP_FB_ACCUM);
  }

  // get frame data
  // TODO: too ugly
  auto fb = reinterpret_cast<const uint8_t *>(
      ospMapFrameBuffer(osp_framebuffer, OSP_FB_COLOR));
  Image image{camera.width, camera.height, 4};
  vector<unsigned char> data(fb, fb + camera.width * camera.height * 4);
  image.data = move(data);
  ospUnmapFrameBuffer(reinterpret_cast<const void *>(fb), osp_framebuffer);

  ospRelease(osp_renderer);
  ospRelease(osp_camera);
  for (auto &light : lights) {
    ospRelease(light);
  }
  ospRelease(osp_framebuffer);
  ospRelease(ospModel);

  const auto delta = chrono::duration_cast<chrono::milliseconds>(
      chrono::steady_clock::now() - start);
  debug.log(to_string(delta.count()) + " ms");

  return image;
}

} // namespace voxer
