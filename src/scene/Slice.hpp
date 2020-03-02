#pragma once
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>
#include <voxer/scene/Slice.hpp>

namespace seria {

template <> inline auto registerObject<voxer::Slice>() {
  using Slice = voxer::Slice;
  return std::make_tuple(member("coef", &Slice::coef),
                         member("volume", &Slice::volume_idx));
}

} // namespace seria