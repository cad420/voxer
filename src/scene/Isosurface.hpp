#pragma once
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>
#include <voxer/scene/Isosurface.hpp>

namespace seria {

template <> inline auto registerObject<voxer::Isosurface>() {
  using Isosurface = voxer::Isosurface;
  return std::make_tuple(member("value", &Isosurface::value),
                         member("volume", &Isosurface::volume_idx),
                         member("render", &Isosurface::render));
}

} // namespace seria
