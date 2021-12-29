#include "Renderers/OSPRay/OSPRayRenderer.hpp"
#include <cmath>
#include <ospray/ospray.h>
#include <ospray/ospray_util.h>
#include <ospray/OSPEnums.h>
#include <spdlog/spdlog.h>
#include <string>
#include <condition_variable>
using namespace std;

namespace {
constexpr float PI = 3.1415926f;

float camera_to_target(const voxer::Camera &camera) {
  auto dx = camera.target[0] - camera.pos[0];
  auto dy = camera.target[1] - camera.pos[1];
  auto dz = camera.target[2] - camera.pos[2];

  auto tmp = dx * dx + dy * dy + dz * dz;
  return std::sqrt(tmp);
}
} // namespace

namespace voxer {

OSPRayRenderer::OSPRayRenderer() {
  m_cache = OSPRayVolumeCache::get_instance();
  spdlog::info("OSPRayRenderer initialized.");
}

OSPRayRenderer::~OSPRayRenderer() { spdlog::info("OSPRayRenderer destroyed."); }

void OSPRayRenderer::set_camera(const Camera &camera) noexcept {
  m_camera = camera;
}

void OSPRayRenderer::add_volume(
    const std::shared_ptr<Volume> &volume) noexcept {
  m_volumes.emplace_back(volume);
}

void OSPRayRenderer::add_isosurface(
    const std::shared_ptr<voxer::Isosurface> &isosurface) noexcept {
  m_isosurfaces.emplace_back(isosurface);
}

void OSPRayRenderer::clear_scene() noexcept {
  m_volumes.clear();
  m_isosurfaces.clear();
}

/**
 * @note ospCommit for osp::Volume is not safe for multi-threads
 */
class Lock{
public:
  Lock(){
    GetOSPRayRendererLock();
  }
  ~Lock(){
    ReleaseOSPRayRendererLock();
  }
private:
  static std::mutex mtx;
  static std::condition_variable cv;
  static bool occupied;
  static void GetOSPRayRendererLock() {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, []() { return !occupied; });
    occupied = true;
  }
  static void ReleaseOSPRayRendererLock() {
    occupied = false;
    cv.notify_one();
  }
};
std::mutex Lock::mtx;
std::condition_variable Lock::cv;
bool Lock::occupied=false;

void OSPRayRenderer::render() {
  Lock lk;
  vector<OSPInstance> osp_instances;

  for (const auto &volume : m_volumes) {
    auto &tfcn = *(volume->tfcn);
    auto data = tfcn.interpolate();
    auto tmp =
        ospNewSharedData(data.first.data(), OSP_FLOAT, data.first.size());
    auto osp_opacity_data = ospNewData(OSP_FLOAT, data.first.size());
    ospCopyData(tmp, osp_opacity_data);
    if(tmp) ospRelease(tmp);
    ospCommit(osp_opacity_data);

    tmp = ospNewSharedData(data.second.data(), OSP_VEC3F, data.second.size());
    auto osp_colors_data = ospNewData(OSP_VEC3F, data.second.size());
    ospCopyData(tmp, osp_colors_data);
    if(tmp) ospRelease(tmp);
    ospCommit(osp_colors_data);

    auto osp_tfcn = ospNewTransferFunction("piecewiseLinear");
    ospSetObject(osp_tfcn, "color", osp_colors_data);
    ospSetObject(osp_tfcn, "opacity", osp_opacity_data);
    ospSetVec2f(osp_tfcn, "valueRange", 0.0f, 255.0f);
    ospCommit(osp_tfcn);

    auto osp_volume = m_cache->get(volume->dataset.get());
    if(!osp_volume) throw std::runtime_error("osp_volume is nullptr");

    ospSetVec3f(osp_volume, "gridSpacing", volume->spacing[0],
                volume->spacing[1], volume->spacing[2]);

    ospCommit(osp_volume);

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

    if(osp_opacity_data) ospRelease(osp_opacity_data);
    if(osp_colors_data) ospRelease(osp_colors_data);
    if(osp_tfcn) ospRelease(osp_tfcn);
    if(osp_volume_model) ospRelease(osp_volume_model);
    if(group) ospRelease(group);

  }

  for (const auto &isosurface : m_isosurfaces) {
    array<float, 2> opacity{1.0f, 1.0f};
    auto tmp = ospNewSharedData(opacity.data(), OSP_FLOAT, opacity.size());
    auto osp_opacity_data = ospNewData(OSP_FLOAT, opacity.size());
    ospCopyData(tmp, osp_opacity_data);
    if(tmp) ospRelease(tmp);
    ospCommit(osp_opacity_data);

    auto osp_volume = m_cache->get(isosurface->dataset.get());

    auto osp_isosurface = ospNewGeometry("isosurface");
    ospSetFloat(osp_isosurface, "isovalue", isosurface->value);
    ospSetParam(osp_isosurface, "volume", OSP_VOLUME, &osp_volume);
    ospCommit(osp_isosurface);

    auto &color = isosurface->color.data;
    auto osp_isosurface_model = ospNewGeometricModel(osp_isosurface);
    ospSetVec4f(osp_isosurface_model, "color", color[0], color[1], color[2],
                1.0f);
    ospCommit(osp_isosurface_model);

    auto osp_group = ospNewGroup();
    ospSetObjectAsData(osp_group, "geometry", OSP_GEOMETRIC_MODEL,
                       osp_isosurface_model);
    ospCommit(osp_group);

    auto osp_instance = ospNewInstance(osp_group);
    ospCommit(osp_instance);

    osp_instances.emplace_back(osp_instance);

    if(osp_isosurface) ospRelease(osp_isosurface);
    if(osp_isosurface_model) ospRelease(osp_isosurface_model);
    if(osp_group) ospRelease(osp_group);
  }

  vector<OSPLight> osp_lights{};

  auto osp_light = ospNewLight("ambient");
  ospCommit(osp_light);
  osp_lights.emplace_back(osp_light);

  auto osp_dir_light = ospNewLight("distant");
  ospSetVec3f(osp_dir_light, "color", 1.0f, 0.0f, 0.0f);
  ospSetVec3f(osp_dir_light, "direction", m_camera.target[0] - m_camera.pos[0],
              m_camera.target[1] - m_camera.pos[1],
              m_camera.target[2] - m_camera.pos[2]);
  ospCommit(osp_dir_light);
  osp_lights.emplace_back(osp_dir_light);

  auto osp_lights_data =
      ospNewSharedData1D(osp_lights.data(), OSP_LIGHT, osp_lights.size());
  ospCommit(osp_lights_data);

  auto osp_world = ospNewWorld();
  auto osp_instance_data = ospNewSharedData1D(
      osp_instances.data(), OSP_INSTANCE, osp_instances.size());
  ospSetObject(osp_world, "instance", osp_instance_data);
  ospSetParam(osp_world, "light", OSP_DATA, &osp_lights_data);
  ospSetObjectAsData(osp_world, "light", OSP_LIGHT, osp_light);
  ospCommit(osp_world);

  auto &camera = m_camera;
  auto osp_camera = ospNewCamera("perspective");
  auto aspect =
      static_cast<float>(camera.width) / static_cast<float>(camera.height);
  if (m_camera.type == Camera::Type::PERSPECTIVE) {
    auto fov = 2 * atan(tan(45.0f * PI / 180.0f / 2.0f) / m_camera.zoom);
    ospSetFloat(osp_camera, "fovy", fov * 180.0f / PI);
  } else {
    osp_camera = ospNewCamera("orthographic");
    auto distance = camera_to_target(m_camera);
    ospSetFloat(osp_camera, "height", distance / aspect / m_camera.zoom);
  }
  ospSetFloat(osp_camera, "aspect", aspect);
  ospSetVec3f(osp_camera, "position", camera.pos[0], camera.pos[1],
              camera.pos[2]);
  ospSetVec3f(osp_camera, "up", camera.up[0], camera.up[1], camera.up[2]);
  ospSetVec3f(osp_camera, "direction", camera.target[0] - camera.pos[0],
              camera.target[1] - camera.pos[1],
              camera.target[2] - camera.pos[2]);
  ospCommit(osp_camera);

  auto osp_renderer = ospNewRenderer("scivis");
  ospSetInt(osp_renderer, "pixelSamples", 1);
  ospSetFloat(osp_renderer, "minContribution", 0.01f);
  ospSetVec3f(osp_renderer, "backgroundColor", m_background[0], m_background[1],
              m_background[2]);
  ospSetInt(osp_renderer, "pixelFilter", OSP_PIXELFILTER_BOX );
  ospSetBool(osp_renderer, "shadows", true);
  ospSetInt(osp_renderer, "aoSamples", 0);
  ospSetFloat(osp_renderer, "volumeSamplingRate", 0.5f);
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
    if(osp_instance) ospRelease(osp_instance);
  }
  if(osp_light) ospRelease(osp_light);
  if(osp_instance_data) ospRelease(osp_instance_data);
  if(osp_renderer) ospRelease(osp_renderer);
  if(osp_camera) ospRelease(osp_camera);
  if(osp_framebuffer) ospRelease(osp_framebuffer);
  if(osp_world) ospRelease(osp_world);
}

auto OSPRayRenderer::get_colors() -> const Image & { return m_image; }

void OSPRayRenderer::set_background(float r, float g, float b) noexcept {
  m_background[0] = r;
  m_background[1] = g;
  m_background[2] = b;
}

bool OSPRayRenderer::has_cache(voxer::StructuredGrid *data) const noexcept {
  return m_cache->has(data);
}

} // namespace voxer

extern "C" {
VoxerIRenderer *voxer_get_renderer() { return new voxer::OSPRayRenderer(); }
}