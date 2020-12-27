#pragma once
#include <seria/object.hpp>
#include <voxer/Data/Image.hpp>

namespace seria {

template <> inline auto register_object<voxer::Image>() {
  return std::make_tuple(member("width", &voxer::Image::width),
                         member("height", &voxer::Image::height),
                         member("data", &voxer::Image::data));
}

} // namespace seria