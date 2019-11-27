#include <voxer/Config.hpp>

using namespace std;

namespace voxer {

IsosurfaceConfig::IsosurfaceConfig(const rapidjson::Value &params) {
  if (!params.HasMember("isovalue") ||
      (!params["isovalue"].IsFloat() && !params["isovalue"].IsInt())) {
    throw "Invalid parameters, should have attribute isovalue for isosurface";
  }

  this->value = params["isovalue"].GetFloat();

  if (!params.HasMember("id") || !params["id"].IsString()) {
    throw "Invalid parameters, should have attribute id for slice";
  }
  this->volumeId = params["id"].GetString();
}
}
