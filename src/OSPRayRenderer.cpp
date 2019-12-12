#include "utils/Debugger.hpp"
#include <algorithm>
#include <chrono>
// #include <ospray/ospray.h>
#include <ospray/ospray_cpp.h>
#include <voxer/OSPRayRenderer.hpp>

using namespace std;

namespace voxer {

static Debugger debug("renderer");

OSPRayRenderer::OSPRayRenderer() {
  auto osp_device = ospNewDevice();
  ospDeviceCommit(osp_device);
  ospSetCurrentDevice(osp_device);
}

auto OSPRayRenderer::render(const Scene &scene) -> Image {
  auto start = chrono::steady_clock::now();

  vector<OSPVolume> osp_volumes;
  vector<uint32_t> render_idxs;
  for (auto i = 0u; i < scene.volumes.size(); i++) {
    const auto &volume = scene.volumes[i];
    const auto &tfcn = scene.tfcns[volume.tfcn_idx];
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

    auto osp_colors_data =
        ospNewData(tfcn.colors.size(), OSP_FLOAT3,
                   reinterpret_cast<const void *>(tfcn.colors.data()),
                   OSP_DATA_SHARED_BUFFER);
    auto osp_opacity_data =
        ospNewData(tfcn.opacities.size(), OSP_FLOAT, tfcn.opacities.data());

    auto osp_tfcn = ospNewTransferFunction("piecewise_linear");
    ospSetData(osp_tfcn, "colors", osp_colors_data);
    ospSetData(osp_tfcn, "opacities", osp_opacity_data);
    ospSet2f(osp_tfcn, "valueRange", 0.0f, 255.0f);
    ospCommit(osp_tfcn);

    auto dataset = scene.datasets[volume.dataset_idx];
    auto osp_dataset =
        ospNewData(dataset->buffer.size(), OSP_UCHAR,
                   static_cast<const void *>(dataset->buffer.data()),
                   OSP_DATA_SHARED_BUFFER);
    ospCommit(osp_dataset);

    auto &meta = dataset->meta;
    auto &dimensions = meta.dimensions;
    auto osp_volume = ospNewVolume("shared_structured_volume");
    ospSet3i(osp_volume, "dimensions", dimensions[0], dimensions[1],
             dimensions[2]);
    ospSetString(osp_volume, "voxelType", "uchar");
    ospSet3f(osp_volume, "gridOrigin",
             -static_cast<float>(dimensions[0]) / 2.0f,
             -static_cast<float>(dimensions[1]) / 2.0f,
             -static_cast<float>(dimensions[2]) / 2.0f);
    ospSetData(osp_volume, "voxelData", osp_dataset);
    ospSetObject(osp_volume, "transferFunction", osp_tfcn);
    ospSet2f(osp_volume, "voxelRange", 0.0f, 255.0f);
    ospSet3f(osp_volume, "gridSpacing", volume.spacing[0], volume.spacing[1],
             volume.spacing[2]);
    ospSet1b(osp_volume, "singleShade", 0);
    ospSet1b(osp_volume, "gradientShadingEnabled", 1);
    ospCommit(osp_volume);

    osp_volumes.push_back(osp_volume);
    if (volume.render) {
      render_idxs.push_back(i);
    }
  }

  auto osp_model = ospNewModel();

  for (auto idx : render_idxs) {
    // https://github.com/ospray/ospray/issues/159#issuecomment-444155750
    // https://github.com/ospray/ospray/pull/165
    // https://github.com/ospray/ospray/issues/159#issuecomment-443847715
    // volume.set("xfm.l.vx", vec3f{0.01, 0.0, 0.0});
    // volume.set("xfm.l.vy", vec3f{0.0, 1.0, 0.0});
    // volume.set("xfm.l.vz", vec3f{0.0, 0.0, 1.0});
    // volume.set("xfm.p", vec3f{0.0, 0.0, 0.0});
    // volume.commit();
    /*
        ospSet3f(osp_volume, "volumeClippingBoxLower", volume.clipBoxLower[0],
                 volume.clipBoxLower[1], volume.clipBoxLower[2]);
        ospSet3f(osp_volume, "volumeClippingBoxUpper", volume.clipBoxUpper[0],
                 volume.clipBoxUpper[1], volume.clipBoxUpper[2]);
        ospCommit(osp_volume);*/
    const auto &osp_volume = osp_volumes[idx];
    ospAddVolume(osp_model, osp_volume);
  }

  for (auto &slice : scene.slices) {
    auto osp_planes_data =
        ospNewData(1, OSP_FLOAT4, static_cast<const void *>(slice.coef.data()));

    auto osp_slice = ospNewGeometry("slices");
    ospSetData(osp_slice, "planes", osp_planes_data);
    ospSetObject(osp_slice, "volume", osp_volumes[slice.volume_idx]);
    ospCommit(osp_slice);

    ospAddGeometry(osp_model, osp_slice);
  }

  for (auto &isosurface : scene.isosurfaces) {
    auto osp_isovalue_data = ospNewData(
        1, OSP_FLOAT, static_cast<const void *>(&(isosurface.value)));

    auto osp_isosurface = ospNewGeometry("isosurfaces");
    ospSetData(osp_isosurface, "isovalues", osp_isovalue_data);
    ospSetObject(osp_isosurface, "volume", osp_volumes[isosurface.volume_idx]);
    ospCommit(osp_isosurface);

    ospAddGeometry(osp_model, osp_isosurface);
  }

  ospCommit(osp_model);

  vector<OSPLight> lights;
  auto osp_light = ospNewLight3("ambient");
  ospSet1f(osp_light, "intensity", 1.25f);
  ospCommit(osp_light);

  lights.push_back(osp_light);

  auto &camera = scene.camera;
  auto osp_camera = ospNewCamera("perspective");
  ospSet1f(osp_camera, "aspect",
           static_cast<float>(camera.width) /
               static_cast<float>(camera.height));
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
  ospSet1f(osp_renderer, "bgColor", 0.0f);
  ospSetObject(osp_renderer, "camera", osp_camera);
  ospSetObject(osp_renderer, "model", osp_model);
  ospCommit(osp_renderer);

  // render frame
  const size_t iteration_times = camera.width == 64 ? 5 : 10;
  auto osp_framebuffer =
      ospNewFrameBuffer(osp::vec2i{static_cast<int>(camera.width),
                                   static_cast<int>(camera.height)},
                        OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
  ospFrameBufferClear(osp_framebuffer, OSP_FB_COLOR | OSP_FB_ACCUM);
  for (int frames = 0; frames < iteration_times; frames++) {
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
  ospRelease(osp_model);

  const auto delta = chrono::duration_cast<chrono::milliseconds>(
      chrono::steady_clock::now() - start);
  debug.log(to_string(delta.count()) + " ms");

  return image;
}

} // namespace voxer
