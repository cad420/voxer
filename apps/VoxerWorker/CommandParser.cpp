#include "CommandParser.hpp"
#include "utils.hpp"
#include <fmt/format.h>
#include <rapidjson/document.h>
#include <stdexcept>
#include <string>
#include <voxer/VolumeRenderer.hpp>
#include <voxer/scene/TransferFunction.hpp>
#include <voxer/utils.hpp>

using namespace voxer;
using namespace std;

static auto create_modifier(rapidjson::Value json_)
    -> pair<string, SceneModifier> {
  if (!json_.IsObject()) {
    throw JSON_error("params", "object");
  }

  auto params = json_.GetObject();
  auto it = params.FindMember("id");
  if (it == params.end() || !it->value.IsString()) {
    throw JSON_error("params.id", "string");
  }
  string id = it->value.GetString();

  auto ptr = make_shared<rapidjson::Value>(move(json_));
  auto modifier = [ptr](const Scene &origin) -> Scene {
    Scene scene = origin;
    auto &json = *ptr;
    auto params = json.GetObject();

    auto it = params.FindMember("camera");
    if (it != params.end() && it->value.IsObject()) {
      auto camera_params = it->value.GetObject();
      it = camera_params.FindMember("width");
      if (it != camera_params.end() && it->value.IsNumber()) {
        scene.camera.width = it->value.GetInt();
      }

      it = camera_params.FindMember("height");
      if (it != camera_params.end() && it->value.IsNumber()) {
        scene.camera.height = it->value.GetInt();
      }

      array<const char *, 3> keys = {"pos", "dir", "up"};
      array<array<float, 3> *, 3> targets = {
          &(scene.camera.pos), &(scene.camera.target), &(scene.camera.up)};
      for (size_t i = 0; i < keys.size(); i++) {
        it = camera_params.FindMember(keys[i]);
        if (it != camera_params.end() && it->value.IsArray()) {
          auto array = it->value.GetArray();
          for (size_t j = 0; j < 3; j++) {
            if (array[i].IsNumber()) {
              (*targets[i])[j] = array[j].GetFloat();
            }
          }
        }
      }
    }

    it = params.FindMember("tfcns");
    if (it != params.end() && it->value.IsArray()) {
      auto tfcn_json = it->value.GetArray();
      for (size_t i = 0; i < tfcn_json.Size(); i++) {
        const auto &item = tfcn_json[i];
        if (item.IsArray()) {
          auto &tfcn = scene.tfcns[i];
          tfcn.clear();
          deserialize_tfcn(tfcn, item);
        }
      }
    }

    // TODO: not needed every time
    for (auto &tfcn : scene.tfcns) {
      for (auto &point : tfcn) {
        point.color = hex_color_to_float(point.hex_color);
      }
    }

    it = params.FindMember("isosurfaces");
    if (it != params.end() && it->value.IsArray()) {
      auto isosurfaces = it->value.GetArray();
      for (size_t i = 0; i < isosurfaces.Size(); i++) {
        auto &item = isosurfaces[i];
        if (item.IsObject()) {
          auto isosurface = item.GetObject();
          it = isosurface.FindMember("value");
          if (it != isosurface.end() && it->value.IsNumber()) {
            scene.isosurfaces[i].value = it->value.GetFloat();
          }
        }
      }
    }

    return scene;
  };

  return make_pair(move(id), move(modifier));
}

auto CommandParser::parse(const char *value, uint64_t size) -> Command {
  document.Parse(value, size);

  if (!document.IsObject()) {
    throw JSON_error("root", "object");
  }

  auto json = document.GetObject();

  auto it = json.FindMember("type");
  if (it == json.end() || !(it->value.IsString())) {
    throw JSON_error("type", "string");
  }
  string command_type = it->value.GetString();

  auto engine = EngineType::OSPRay;
  it = json.FindMember("engine");
  if (it != json.end() && it->value.IsString()) {
    auto type = it->value.GetString();
    if (strcmp(type, "OpenGL") == 0) {
      engine = EngineType::OpenGL;
    } else if (strcmp(type, "OSPRay") == 0) {
      engine = EngineType::OSPRay;
    } else {
      throw runtime_error("Unsupported rendering engine: " + string(type));
    }
  }

  it = json.FindMember("params");
  if (it == json.end()) {
    throw JSON_error("params", "required");
  }
  auto params = it->value.GetObject();

  if (command_type == "render") {
    return {Command::Type::Render, engine, Scene::deserialize(params)};
  }

  if (command_type == "query") {
    if (it == json.end() || !it->value.IsObject()) {
      throw JSON_error("params", "object");
    }

    it = params.FindMember("target");
    if (it == params.end() || !it->value.IsString()) {
      throw JSON_error("params.target", "string");
    }

    string target = it->value.GetString();
    if (target == "datasets") {
      return {Command::Type::QueryDatasets, engine, nullptr};
    }

    if (target == "pipelines") {
      return {Command::Type::QueryPipelines, engine, nullptr};
    }

    if (target == "dataset") {
      return {Command::Type::QueryDataset, engine,
              SceneDataset::deserialize(params)};
    }

    if (target == "pipeline") {
      it = params.FindMember("id");
      if (it == params.end()) {
        throw JSON_error("params.id", "string when params.target == pipeline");
      }
      return {Command::Type::QueryPipeline, engine, it->value.GetString()};
    }
  }

  if (command_type == "save") {
    return {Command::Type::Save, engine,
            make_pair(extract_params(string(value, size)),
                      Scene::deserialize(params))};
  }

  if (command_type == "run") {
    return {Command::Type::RunPipeline, engine, create_modifier(move(params))};
  }

  if (command_type == "modify") {
    it = params.FindMember("id");
    if (it == params.end() || !it->value.IsString()) {
      throw JSON_error("params.id", "string");
    }

    return {Command::Type::ModifyDataset, engine,
            make_pair(string(it->value.GetString()),
                      Scene::deserialize(params))};
  }

  if (command_type == "add") {
    it = params.FindMember("target");
    if (it == params.end() || !it->value.IsString()) {
      throw JSON_error("params.target", "string");
    }

    string target = it->value.GetString();
    if (target == "dataset") {
      return {Command::Type::AddDataset, engine,
              extract_params(string(value, size))};
    }

    throw runtime_error("unsupported add type: " + target);
  }

  throw runtime_error("invalid JSON: unknown command type");
}

Command CommandParser::parse(const string &value) {
  return this->parse(value.data(), value.size());
}
