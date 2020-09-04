#include "Rendering/OSPRay/OSPRayRenderer.hpp"
#include <iostream>
#include <ospray/ospray.h>
#include <ospray/ospray_util.h>
#include <stdexcept>

using namespace std;

namespace voxer {

OSPRayRenderer::OSPRayRenderer() {
  ospInit();
  osp_device = ospGetCurrentDevice();
  //  ospLoadModule("ispc");
  //  osp_device = ospNewDevice("cpu");
  //  if (osp_device == nullptr) {
  //    throw runtime_error("Failed to initialize OSPRay");
  //  }
  //#ifndef NDEBUG
  ////  auto logLevel = OSP_LOG_INFO;
  ////  ospDeviceSetParam(osp_device, "logLevel", OSP_INT, &logLevel);
  //  ospDeviceSetParam(osp_device, "logOutput", OSP_STRING, "cout");
  //  ospDeviceSetParam(osp_device, "errorOutput", OSP_STRING, "cerr");
  //#endif
  //  ospDeviceCommit(osp_device);
  //  ospSetCurrentDevice(osp_device);
  std::cout << "OSPRay initialized" << std::endl;
}

OSPRayRenderer::~OSPRayRenderer() {
  if (osp_device == nullptr)
    return;

  ospDeviceRelease(osp_device);
}

void OSPRayRenderer::set_camera(const Camera &camera) { m_camera = camera; }

void OSPRayRenderer::add_volume(const std::shared_ptr<Volume> &volume) {
  m_volumes.emplace_back(volume);
}

void OSPRayRenderer::add_isosurface(
    const std::shared_ptr<voxer::Isosurface> &isosurface) {
  m_isosurfaces.emplace_back(isosurface);
}

void OSPRayRenderer::clear_scene() {
  m_volumes.clear();
  m_isosurfaces.clear();
}

void OSPRayRenderer::render() {
  vector<OSPInstance> osp_instances;

  for (const auto &volume : m_volumes) {
    const auto &tfcn = *(volume->tfcn);
    auto tmp = ospNewSharedData(tfcn.opacities.data(), OSP_FLOAT,
                                tfcn.opacities.size());
    auto osp_opacity_data = ospNewData(OSP_FLOAT, tfcn.opacities.size());
    ospCopyData(tmp, osp_opacity_data);
    ospRelease(tmp);
    ospCommit(osp_opacity_data);

    tmp = ospNewSharedData(tfcn.colors.data(), OSP_VEC3F, tfcn.colors.size());
    auto osp_colors_data = ospNewData(OSP_VEC3F, tfcn.colors.size());
    ospCopyData(tmp, osp_colors_data);
    ospRelease(tmp);
    ospCommit(osp_colors_data);

    auto osp_tfcn = ospNewTransferFunction("piecewiseLinear");
    ospSetObject(osp_tfcn, "color", osp_colors_data);
    ospSetObject(osp_tfcn, "opacity", osp_opacity_data);
    ospSetVec2f(osp_tfcn, "valueRange", 0.0f, 255.0f);
    ospCommit(osp_tfcn);

    auto &osp_volume = this->get_osp_volume(volume->dataset.get());

    auto osp_volume_model = ospNewVolumetricModel(osp_volume);
    ospSetParam(osp_volume_model, "transferFunction", OSP_TRANSFER_FUNCTION,
                &osp_tfcn);
    ospCommit(osp_volume_model);

    // TODO: handle clip
    auto group = ospNewGroup();
    ospSetObjectAsData(group, "volume", OSP_VOLUMETRIC_MODEL, osp_volume_model);
    ospCommit(group);

    // TODO: handle scale
    auto osp_instance = ospNewInstance(group);
    ospCommit(osp_instance);
    osp_instances.emplace_back(osp_instance);

    ospRelease(osp_opacity_data);
    ospRelease(osp_colors_data);
    ospRelease(osp_tfcn);
    ospRelease(osp_volume_model);
    ospRelease(group);
  }

  for (const auto &isosurface : m_isosurfaces) {
    array<float, 2> opacity{1.0f, 1.0f};
    auto tmp = ospNewSharedData(opacity.data(), OSP_FLOAT, opacity.size());
    auto osp_opacity_data = ospNewData(OSP_FLOAT, opacity.size());
    ospCopyData(tmp, osp_opacity_data);
    ospRelease(tmp);
    ospCommit(osp_opacity_data);

    auto &color = isosurface->color.data;
    array<array<float, 3>, 2> colors{color, color};
    tmp = ospNewSharedData(colors.data(), OSP_VEC3F, colors.size());
    auto osp_colors_data = ospNewData(OSP_VEC3F, colors.size());
    ospCopyData(tmp, osp_colors_data);
    ospRelease(tmp);
    ospCommit(osp_colors_data);

    auto osp_tfcn = ospNewTransferFunction("piecewiseLinear");
    ospSetObject(osp_tfcn, "color", osp_colors_data);
    ospSetObject(osp_tfcn, "opacity", osp_opacity_data);
    ospSetVec2f(osp_tfcn, "valueRange", 0.0f, 255.0f);
    ospCommit(osp_tfcn);

    auto &osp_volume = this->get_osp_volume(isosurface->dataset.get());
    auto osp_volume_model = ospNewVolumetricModel(osp_volume);
    ospSetParam(osp_volume_model, "transferFunction", OSP_TRANSFER_FUNCTION,
                &osp_tfcn);
    ospCommit(osp_volume_model);

    auto osp_isosurface = ospNewGeometry("isosurface");
    ospSetFloat(osp_isosurface, "isovalue", isosurface->value);
    ospSetParam(osp_isosurface, "volume", OSP_VOLUMETRIC_MODEL,
                &osp_volume_model);
    ospCommit(osp_isosurface);

    auto osp_isosurface_model = ospNewGeometricModel(osp_isosurface);
    ospCommit(osp_isosurface_model);

    auto osp_group = ospNewGroup();
    ospSetObjectAsData(osp_group, "geometry", OSP_GEOMETRIC_MODEL,
                       osp_isosurface_model);
    ospCommit(osp_group);

    auto osp_instance = ospNewInstance(osp_group);
    ospCommit(osp_instance);

    osp_instances.emplace_back(osp_instance);

    ospRelease(osp_tfcn);
    ospRelease(osp_volume_model);
    ospRelease(osp_isosurface);
    ospRelease(osp_isosurface_model);
    ospRelease(osp_group);
  }

  auto osp_light = ospNewLight("ambient");
  ospCommit(osp_light);

  auto osp_world = ospNewWorld();
  auto osp_instance_data = ospNewSharedData1D(
      osp_instances.data(), OSP_INSTANCE, osp_instances.size());
  ospSetObject(osp_world, "instance", osp_instance_data);
  ospSetObjectAsData(osp_world, "light", OSP_LIGHT, osp_light);
  ospCommit(osp_world);

  auto &camera = m_camera;
  auto osp_camera = ospNewCamera("perspective");
  ospSetFloat(osp_camera, "fovy", 45.0f);
  ospSetFloat(osp_camera, "aspect",
              static_cast<float>(camera.width) /
                  static_cast<float>(camera.height));
  ospSetVec3f(osp_camera, "position", camera.pos[0], camera.pos[1],
              camera.pos[2]);
  ospSetVec3f(osp_camera, "up", camera.up[0], camera.up[1], camera.up[2]);
  ospSetVec3f(osp_camera, "direction", camera.target[0] - camera.pos[0],
              camera.target[1] - camera.pos[1],
              camera.target[2] - camera.pos[2]);
  ospCommit(osp_camera);

  auto osp_renderer = ospNewRenderer("scivis");
  ospSetInt(osp_renderer, "pixelSamples", 1);
  ospSetInt(osp_renderer, "aoSamples", 0);
  ospSetFloat(osp_renderer, "volumeSamplingRate", 0.125f);
  ospSetFloat(osp_renderer, "minContribution", 0.01f);
  ospSetFloat(osp_renderer, "backgroundColor", 0.0f);
  ospCommit(osp_renderer);

  const size_t iteration_times = camera.width == 64 ? 1 : 8;
  auto osp_framebuffer = ospNewFrameBuffer(
      camera.width, camera.height, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
  ospResetAccumulation(osp_framebuffer);
  for (size_t frames = 0; frames < iteration_times; frames++) {
    ospRenderFrameBlocking(osp_framebuffer, osp_renderer, osp_camera,
                           osp_world);
  }

  auto fb = reinterpret_cast<const uint8_t *>(
      ospMapFrameBuffer(osp_framebuffer, OSP_FB_COLOR));
  // TODO: better way to get framebuffer data
  vector<unsigned char> data(fb, fb + camera.width * camera.height * 4);
  m_image.width = camera.width;
  m_image.height = camera.height;
  m_image.channels = 4;
  m_image.data = move(data);
  ospUnmapFrameBuffer(reinterpret_cast<const void *>(fb), osp_framebuffer);

  for (auto &osp_instance : osp_instances) {
    ospRelease(osp_instance);
  }
  ospRelease(osp_light);
  ospRelease(osp_instance_data);
  ospRelease(osp_renderer);
  ospRelease(osp_camera);
  ospRelease(osp_framebuffer);
  ospRelease(osp_world);
}

auto OSPRayRenderer::get_colors() -> const Image & { return m_image; }

OSPVolume &OSPRayRenderer::create_osp_volume(StructuredGrid *dataset) {
  auto &info = dataset->info;
  auto &dimensions = info.dimensions;

  // TODO: handle datasets created by differing
  auto osp_volume_data =
      ospNewSharedData(dataset->buffer.data(), OSP_UCHAR, dimensions[0], 0,
                       dimensions[1], 0, dimensions[2], 0);
  ospCommit(osp_volume_data);

  auto osp_volume = ospNewVolume("structuredRegular");
  ospSetObject(osp_volume, "data", osp_volume_data);
  ospSetVec3f(osp_volume, "gridOrigin",
              -static_cast<float>(dimensions[0]) / 2.0f,
              -static_cast<float>(dimensions[1]) / 2.0f,
              -static_cast<float>(dimensions[2]) / 2.0f);
  // TODO: handle customized spacing
  ospSetVec3f(osp_volume, "gridSpacing", 1.0f, 1.0f, 1.0f);
  ospCommit(osp_volume);

  auto res = m_osp_volume_cache.emplace(dataset, osp_volume);
  return res.first->second;
}

OSPVolume &OSPRayRenderer::get_osp_volume(StructuredGrid *dataset) {
  auto it = this->m_osp_volume_cache.find(dataset);
  if (it == this->m_osp_volume_cache.end()) {
    return this->create_osp_volume(dataset);
  }
  return it->second;
}

} // namespace voxer