#include "OSPRayVolumeCache.hpp"
#include <ospray/ospray_util.h>

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

} // namespace voxer