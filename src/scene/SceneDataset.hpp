#pragma once
#include <seria/deserialize.hpp>
#include <seria/serialize.hpp>
#include <voxer/scene/SceneDataset.hpp>

namespace seria {

template <> inline auto registerObject<voxer::SceneDataset>() {
  using SceneDataset = voxer::SceneDataset;
  return std::make_tuple(member("name", &SceneDataset::name),
                         member("variable", &SceneDataset::variable),
                         member("timestep", &SceneDataset::timestep));
}

} // namespace seria