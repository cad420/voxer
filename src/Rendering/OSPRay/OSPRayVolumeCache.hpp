#pragma once
#include <mutex>
#include <ospray/ospray.h>
#include <unordered_map>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

class OSPRayVolumeCache {
public:
  static auto get_instance() noexcept -> OSPRayVolumeCache *;
  bool has(StructuredGrid *data);
  auto get(StructuredGrid *data) -> OSPVolume;
  void load(StructuredGrid *data);

private:
  OSPRayVolumeCache() noexcept = default;

  std::mutex m_mutex;
  std::unordered_map<StructuredGrid *, OSPVolume> m_cache{};
};

} // namespace voxer
