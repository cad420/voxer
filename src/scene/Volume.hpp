#pragma once
#include "TransferFunction.hpp"
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>
#include <voxer/scene/Volume.hpp>

namespace seria {

template <> inline auto registerObject<voxer::Volume>() {
  using Volume = voxer::Volume;
  return std::make_tuple(member("dataset", &Volume::dataset_idx),
                         member("tfcn", &Volume::tfcn_idx),
                         member("spacing", &Volume::spacing),
                         member("render", &Volume::render));
}

} // namespace seria