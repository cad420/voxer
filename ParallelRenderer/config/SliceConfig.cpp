#include "SliceConfig.h"

using namespace std;

SliceConfig::SliceConfig(const rapidjson::Value &params) {
  if (!params.HasMember("coeff") || !params["coeff"].IsArray()) {
    throw "Invalid parameters, should have attribute coeff for slice";
  }

  auto coeff = params["coeff"].GetArray();
  if (coeff.Size() != 4) {
    throw "Invalid parameters, should have attribute coeff.length = 4 for slice";
  }
  this->a = coeff[0].GetFloat();
  this->b = coeff[1].GetFloat();
  this->c = coeff[2].GetFloat();
  this->d = coeff[3].GetFloat();

  if (!params.HasMember("id") || !params["id"].IsString()) {
    throw "Invalid parameters, should have attribute id for slice";
  }
  this->volumeId = params["id"].GetString();

  if (!params.HasMember("id") || !params["id"].IsString()) {
    throw "Invalid parameters, should have attribute id for slice";
  }
}
