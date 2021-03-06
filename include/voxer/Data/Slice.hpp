#pragma once
#include <string>
#include <vector>
#include <voxer/Data/Annotation.hpp>
#include <voxer/Data/Image.hpp>
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

struct SliceInfo {
  StructuredGrid::Axis axis = StructuredGrid::Axis::X;
  uint32_t index = 0;

  auto operator==(const SliceInfo &rhs) const -> bool {
    return axis == rhs.axis && index == rhs.index;
  }

  auto operator!=(const SliceInfo &rhs) const -> bool {
    return !(*this == rhs);
  }
};

struct AnnotatedSliceInfo : public SliceInfo {
  std::vector<Annotation> annotations;
};

struct ImageSlice : public AnnotatedSliceInfo, public Image {};

} // namespace voxer

namespace std {
template <> struct hash<voxer::SliceInfo> {
  size_t operator()(const voxer::SliceInfo &s) const {
    // http://stackoverflow.com/a/1646913/126995
    size_t res = 17;
    res = res * 31 + hash<voxer::StructuredGrid::Axis>()(s.axis);
    res = res * 31 + hash<uint32_t>()(s.index);
    return res;
  }
};
} // namespace std
