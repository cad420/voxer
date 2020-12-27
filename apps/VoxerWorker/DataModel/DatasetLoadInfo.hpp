#pragma once
#include "DataModel/DatasetInfo.hpp"
#include <array>
#include <seria/object.hpp>

namespace voxer::remote {

struct DatasetLoadInfo {
  DatasetID id;
  std::string name;
  std::string path;

  bool operator==(const DatasetLoadInfo &other) const { return id == other.id; }
};

} // namespace voxer::remote

namespace seria {

template <> inline auto register_object<voxer::remote::DatasetLoadInfo>() {
  using DatasetLoadInfo = voxer::remote::DatasetLoadInfo;
  return std::make_tuple(member("id", &DatasetLoadInfo::id),
                         member("name", &DatasetLoadInfo::name),
                         member("path", &DatasetLoadInfo::path));
}

} // namespace seria

namespace std {

template <> struct hash<voxer::remote::DatasetLoadInfo> {
  size_t operator()(const voxer::remote::DatasetLoadInfo &s) const {
    // Compute individual hash values for first, second and third
    // http://stackoverflow.com/a/1646913/126995
    size_t res = 17;
    res = res * 31 + hash<string>()(s.id);
    res = res * 31 + hash<string>()(s.name);
    res = res * 31 + hash<string>()(s.path);
    return res;
  }
};

} // namespace std