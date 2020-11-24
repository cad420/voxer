#pragma once
#include "Common/LRUCache.hpp"
#include <mutex>
#include <ospray/ospray.h>
#include <unordered_map>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

class OSPRayManagedResource {
public:
  OSPRayManagedResource(OSPVolume volume,
                        std::shared_ptr<voxer::StructuredGrid> data)
      : m_data(std::move(data)), m_volume(volume) {}

  ~OSPRayManagedResource() {
    if (m_volume == nullptr) {
      return;
    }

    ospRelease(m_volume);
  }

  OSPVolume value() const noexcept { return m_volume; }

  OSPRayManagedResource &operator=(OSPRayManagedResource &&rhs) noexcept {
    if (this == &rhs) {
      return *this;
    }
    m_data = std::move(rhs.m_data);
    m_volume = rhs.m_volume;
    rhs.m_volume = nullptr;
    return *this;
  }

  OSPRayManagedResource(OSPRayManagedResource &&rhs) noexcept
      : m_data(std::move(rhs.m_data)), m_volume(rhs.m_volume) {
    rhs.m_volume = nullptr;
  }

private:
  std::shared_ptr<voxer::StructuredGrid> m_data;
  OSPVolume m_volume;
};

class OSPRayVolumeCache {
public:
  static auto get_instance() noexcept -> OSPRayVolumeCache *;
  bool has(StructuredGrid *data) noexcept;
  auto get(StructuredGrid *data) -> OSPVolume;
  void load(StructuredGrid *data);

private:
  OSPRayVolumeCache() noexcept = default;

  std::mutex m_mutex;
  LRUCache<StructuredGrid *, OSPRayManagedResource> m_cache{};
};

} // namespace voxer
