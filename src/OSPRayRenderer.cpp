#include "utils/Debugger.hpp"
#include <algorithm>
#include <chrono>
// #include <ospray/ospray.h>
#include <memory>
#include <ospray/ospray_cpp.h>
#include <voxer/OSPRayRenderer.hpp>

using namespace std;

namespace voxer {

static Debugger debug("renderer");

static auto interpolate(const TransferFunction &tf)
    -> pair<std::vector<float>, vector<array<float, 3>>> {
  static const size_t total_samples = 200;

  vector<float> opacities;
  opacities.reserve(total_samples);
  vector<array<float, 3>> colors(total_samples);
  colors.reserve(total_samples);

  for (int32_t i = 0; i < (tf.stops.size() - 1); ++i) {
    auto start_x = tf.stops[i];
    auto end_x = tf.stops[i + 1];
    auto start_opacity = tf.opacities[i];
    auto end_opacity = tf.opacities[i + 1];
    auto start_r = tf.colors[i][0];
    auto start_g = tf.colors[i][1];
    auto start_b = tf.colors[i][2];
    auto end_r = tf.colors[i + 1][0];
    auto end_g = tf.colors[i + 1][1];
    auto end_b = tf.colors[i + 1][2];

    auto samples = static_cast<uint32_t>(total_samples * (end_x - start_x));
    auto delta = 1.0f / static_cast<float>(samples);
    auto diff_opacity = delta * (end_opacity - start_opacity);
    auto diff_r = delta * (end_r - start_r);
    auto diff_g = delta * (end_g - start_g);
    auto diff_b = delta * (end_b - start_b);
    for (auto j = 0; j < samples; j++) {
      opacities.emplace_back(start_opacity + j * diff_opacity);
      colors.emplace_back(array<float, 3>{
          start_r + j * diff_r, start_g + j * diff_g, start_b + j * diff_b});
    }
  }

  return make_pair(move(opacities), move(colors));
}

struct OSPRayRenderer::Impl {
  explicit Impl(const std::vector<Dataset> &datasets) {
    for (auto &dataset : datasets) {
      auto &meta = dataset.meta;
      auto &dimensions = meta.dimensions;
      auto osp_volume = ospNewVolume("block_bricked_volume");
      ospSet3i(osp_volume, "dimensions", dimensions[0], dimensions[1],
               dimensions[2]);
      ospSetString(osp_volume, "voxelType", "uchar");
      ospSetRegion(osp_volume,
                   reinterpret_cast<void *>(
                       const_cast<uint8_t *>(dataset.buffer.data())),
                   osp::vec3i{0, 0, 0},
                   osp::vec3i{static_cast<int>(dimensions[0]),
                              static_cast<int>(dimensions[1]),
                              static_cast<int>(dimensions[2])});
      ospCommit(osp_volume);
      osp_volumes.emplace(&dataset, osp_volume);
    }
  }
  std::map<const Dataset *, OSPVolume> osp_volumes;
};

OSPRayRenderer::OSPRayRenderer(const DatasetStore &datasets)
    : Renderer(datasets) {
  auto osp_device = ospNewDevice();
  //  ospDeviceSet1i(osp_device, "logLevel", 2);
  ospDeviceSetString(osp_device, "logOutput", "cout");
  ospDeviceSetString(osp_device, "errorOutput", "cout");
  ospDeviceCommit(osp_device);
  ospSetCurrentDevice(osp_device);

  this->impl = make_unique<OSPRayRenderer::Impl>(datasets.get());
}

OSPRayRenderer::~OSPRayRenderer() = default;

auto OSPRayRenderer::render(const Scene &scene) -> Image {
  auto start = chrono::steady_clock::now();

  vector<OSPVolume> osp_volumes;
  vector<uint32_t> render_idxs;
  for (auto i = 0u; i < scene.volumes.size(); i++) {
    const auto &volume = scene.volumes[i];
    const auto &tfcn = scene.tfcns[volume.tfcn_idx];
    auto interpolated = interpolate(tfcn);
    auto osp_colors_data =
        ospNewData(tfcn.colors.size(), OSP_FLOAT3,
                   reinterpret_cast<const void *>(interpolated.second.data()),
                   OSP_DATA_SHARED_BUFFER);
    auto osp_opacity_data =
        ospNewData(tfcn.opacities.size(), OSP_FLOAT, interpolated.first.data(),
                   OSP_DATA_SHARED_BUFFER);
    auto osp_tfcn = ospNewTransferFunction("piecewise_linear");
    ospSetData(osp_tfcn, "colors", osp_colors_data);
    ospSetData(osp_tfcn, "opacities", osp_opacity_data);
    ospSet2f(osp_tfcn, "valueRange", 0.0f, 255.0f);
    ospCommit(osp_tfcn);

    auto &scene_dataset = scene.datasets[volume.dataset_idx];
    auto &dataset = datasets.get(scene_dataset);
    auto &meta = dataset.meta;
    auto &dimensions = meta.dimensions;
    auto osp_volume = this->impl->osp_volumes.at(&dataset);
    ospSet2f(osp_volume, "voxelRange", 0.0f, 255.0f);
    ospSet3f(osp_volume, "gridOrigin",
             -static_cast<float>(dimensions[0]) / 2.0f,
             -static_cast<float>(dimensions[1]) / 2.0f,
             -static_cast<float>(dimensions[2]) / 2.0f);
    ospSetObject(osp_volume, "transferFunction", osp_tfcn);
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
  ospSet1i(osp_renderer, "spp", 1);
  ospSet1i(osp_renderer, "aoSamples", 1);
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
