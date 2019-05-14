#include "voxer/configs/CameraConfig.hpp"

using namespace std;

string err(string attri, string parent) {
  return "Invalid parameters, should have attribute `" + attri + "` for " +
         parent;
}

CameraConfig::CameraConfig(const rapidjson::Value &params) {
  if (!params.HasMember("type") || !params["type"].IsString()) {
    throw err("type", "camera");
  }
  this->type = params["type"].GetString();
  if (!params.HasMember("pos") || !params["pos"].IsArray()) {
    throw err("pos", "camera");
  }
  auto pos = params["pos"].GetArray();
  if (pos.Size() != 3) {
    throw err("pos.length", "camera");
  }
  this->pos.x = pos[0].GetFloat();
  this->pos.y = pos[1].GetFloat();
  this->pos.z = pos[2].GetFloat();

  auto up = params["up"].GetArray();
  if (up.Size() != 3) {
    throw err("up.length", "camera");
  }
  this->up.x = up[0].GetFloat();
  this->up.y = up[1].GetFloat();
  this->up.z = up[2].GetFloat();

  auto dir = params["dir"].GetArray();
  if (dir.Size() != 3) {
    throw err("dir.length", "camera");
  }
  this->dir.x = dir[0].GetFloat();
  this->dir.y = dir[1].GetFloat();
  this->dir.z = dir[2].GetFloat();
}

CameraConfig::CameraConfig(const CameraConfig &exist,
                           const std::map<std::string, std::string> &params) {
  this->type = exist.type;
  this->pos = exist.pos;
  this->up = exist.up;
  this->dir = exist.dir;
  if (params.find("pos.x") != params.end()) {
    this->pos.x = stof(params.at("pos.x"));
  }
  if (params.find("pos.y") != params.end()) {
    this->pos.y = stof(params.at("pos.y"));
  }
  if (params.find("pos.z") != params.end()) {
    this->pos.z = stof(params.at("pos.z"));
  }
  if (params.find("up.x") != params.end()) {
    this->up.x = stof(params.at("up.x"));
  }
  if (params.find("up.y") != params.end()) {
    this->up.y = stof(params.at("up.y"));
  }
  if (params.find("up.z") != params.end()) {
    this->up.z = stof(params.at("up.z"));
  }
  if (params.find("dir.x") != params.end()) {
    this->dir.x = stof(params.at("dir.x"));
  }
  if (params.find("dir.y") != params.end()) {
    this->dir.y = stof(params.at("dir.y"));
  }
  if (params.find("dir.z") != params.end()) {
    this->dir.z = stof(params.at("dir.z"));
  }
}

CameraConfig::CameraConfig(const CameraConfig &exist,
                           const rapidjson::Value &params) {
  this->type = exist.type;
  this->pos = exist.pos;
  this->up = exist.up;
  this->dir = exist.dir;
  if (params.HasMember("pos") || params["pos"].IsArray()) {
    auto pos = params["pos"].GetArray();
    if (pos.Size() == 3) {
      this->pos.x = pos[0].GetFloat();
      this->pos.y = pos[1].GetFloat();
      this->pos.z = pos[2].GetFloat();
    }
  }
  if (params.HasMember("up") || params["up"].IsArray()) {
    auto up = params["up"].GetArray();
    if (up.Size() == 3) {
      this->up.x = up[0].GetFloat();
      this->up.y = up[1].GetFloat();
      this->up.z = up[2].GetFloat();
    }
  }
  if (params.HasMember("dir") || params["dir"].IsArray()) {
    auto dir = params["dir"].GetArray();
    if (dir.Size() == 3) {
      this->dir.x = dir[0].GetFloat();
      this->dir.y = dir[1].GetFloat();
      this->dir.z = dir[2].GetFloat();
    }
  }
}