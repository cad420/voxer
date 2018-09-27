#include "IsosurfaceConfig.h"

using namespace std;

IsosurfaceConfig::IsosurfaceConfig(const rapidjson::Value &params) {
  if (!params.HasMember("value") ||
      (!params["value"].IsFloat() && !params["value"].IsInt())) {
    throw "Invalid parameters, should have attribute value for isosurface";
  }

  this->value = params["value"].GetFloat();

  if (!params.HasMember("id") || !params["id"].IsString()) {
    throw "Invalid parameters, should have attribute id for slice";
  }
  this->volumeId = params["id"].GetString();
}
