#include "OSPRayVolumeCache.hpp"
#include <ospray/ospray_util.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace {

OSPVolume create_osp_volume(voxer::StructuredGrid *dataset) {
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

  return osp_volume;
}

} // namespace

namespace voxer {

OSPRayVolumeCache::OSPRayVolumeCache() {
  ospLoadModule("ispc");
  m_osp_device = ospNewDevice("cpu");
  if (m_osp_device == nullptr) {
    throw std::runtime_error("Failed to initialize OSPRay");
  }
#ifndef NDEBUG
  auto logLevel = OSP_LOG_DEBUG;
  ospDeviceSetParam(m_osp_device, "logLevel", OSP_INT, &logLevel);
  ospDeviceSetParam(m_osp_device, "logOutput", OSP_STRING, "cout");
  ospDeviceSetParam(m_osp_device, "errorOutput", OSP_STRING, "cerr");
#endif
  ospDeviceCommit(m_osp_device);
  ospSetCurrentDevice(m_osp_device);

  auto major_version =
      ospDeviceGetProperty(m_osp_device, OSP_DEVICE_VERSION_MAJOR);
  auto minor_version =
      ospDeviceGetProperty(m_osp_device, OSP_DEVICE_VERSION_MINOR);
  spdlog::info("OSPRayRenderer initialized, OSPRay version {}.{}.",
               major_version, minor_version);
}

OSPRayVolumeCache::~OSPRayVolumeCache() noexcept {
  if (m_osp_device != nullptr) {
    ospDeviceRelease((m_osp_device));
  }
}

auto OSPRayVolumeCache::get_instance() noexcept -> OSPRayVolumeCache * {
  static OSPRayVolumeCache instance{};

  return &instance;
}

auto OSPRayVolumeCache::get(StructuredGrid *data) -> OSPVolume {
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_cache.has(data)) {
    return m_cache.get(data)->value();
  }

  lock.unlock();
  auto volume = create_osp_volume(data);

  lock.lock();
  m_cache.emplace(data,
                  OSPRayManagedResource(volume, data->shared_from_this()));

  return volume;
}

void OSPRayVolumeCache::load(StructuredGrid *data) {
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_cache.has(data)) {
    return;
  }

  auto volume = create_osp_volume(data);

  m_cache.emplace(data,
                  OSPRayManagedResource(volume, data->shared_from_this()));
}

bool OSPRayVolumeCache::has(StructuredGrid *data) noexcept {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_cache.has(data);
}

OSPDevice OSPRayVolumeCache::get_device() const noexcept {
  return m_osp_device;
}

} // namespace voxer