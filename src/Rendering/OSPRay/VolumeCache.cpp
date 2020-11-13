#include "VolumeCache.hpp"
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

auto VolumeCache::create() noexcept -> VolumeCache * {
  static VolumeCache instance{};

  return &instance;
}

auto VolumeCache::get(StructuredGrid *data) -> OSPVolume {
  std::unique_lock<std::mutex> lock(m_mutex);

  auto it = m_cache.find(data);
  if (it != m_cache.end()) {
    return it->second;
  }

  lock.unlock();
  auto volume = create_osp_volume(data);

  lock.lock();
  m_cache.emplace(data, volume);

  return volume;
}

void VolumeCache::load(StructuredGrid *data) {
  std::unique_lock<std::mutex> lock(m_mutex);

  auto it = m_cache.find(data);
  if (it != m_cache.end()) {
    return;
  }

  lock.unlock();
  auto volume = create_osp_volume(data);

  lock.lock();
  m_cache.emplace(data, volume);
}

} // namespace voxer