#pragma once
#include <mutex>
#include <ospray/ospray.h>
#include <unordered_map>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

class VolumeCache {
public:
  static auto create() noexcept -> VolumeCache *;

  auto get(StructuredGrid *data) -> OSPVolume;
  void load(StructuredGrid *data);

private:
  VolumeCache() noexcept = default;

  std::mutex m_mutex;
  std::unordered_map<StructuredGrid *, OSPVolume> m_cache{};
};

} // namespace voxer
