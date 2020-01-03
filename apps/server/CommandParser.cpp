#include "CommandParser.hpp"
#include "utils.hpp"
#include <fmt/format.h>
#include <rapidjson/document.h>
#include <stdexcept>
#include <string>
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
          &(scene.camera.pos), &(scene.camera.dir), &(scene.camera.up)};
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
      scene.tfcns.clear();
      for (auto &item : tfcn_json) {
        TransferFunction tfcn{};
        if (item.IsArray()) {
          auto points = item.GetArray();
          for (auto &point_json : points) {
            ControlPoint point{};
            formatter::deserialize(point, point_json);
            tfcn.emplace_back(point);
          }
        }
        scene.tfcns.emplace_back(move(tfcn));
      }
    }

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

  it = json.FindMember("params");
  if (it == json.end()) {
    throw JSON_error("params", "required");
  }
  auto params = it->value.GetObject();

  if (command_type == "render") {
    return {Command::Type::Render, Scene::deserialize(params)};
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
      return {Command::Type::QueryDatasets, nullptr};
    }

    if (target == "pipelines") {
      return {Command::Type::QueryPipelines, nullptr};
    }

    if (target == "dataset") {
      return {Command::Type::QueryDataset, SceneDataset::deserialize(params)};
    }

    if (target == "pipeline") {
      it = params.FindMember("id");
      if (it == params.end()) {
        throw JSON_error("params.id", "string when params.target == pipeline");
      }
      return {Command::Type::QueryPipeline, it->value.GetString()};
    }
  }

  if (command_type == "save") {
    return {Command::Type::Save, make_pair(extract_params(string(value, size)),
                                           Scene::deserialize(params))};
  }

  if (command_type == "run") {
    return {Command::Type::RunPipeline, create_modifier(move(params))};
  }

  if (command_type == "modify") {
    it = params.FindMember("id");
    if (it == params.end() || !it->value.IsString()) {
      throw JSON_error("params.id", "string");
    }

    return {
        Command::Type::ModifyDataset,
        make_pair(string(it->value.GetString()), Scene::deserialize(params))};
  }

  if (command_type == "add") {
    it = params.FindMember("target");
    if (it == params.end() || !it->value.IsString()) {
      throw JSON_error("params.target", "string");
    }

    string target = it->value.GetString();
    if (target == "dataset") {
      return {Command::Type::AddDataset, extract_params(string(value, size))};
    }

    throw runtime_error("unsupported add type: " + target);
  }

  throw runtime_error("invalid JSON: unknown command type");
}

Command CommandParser::parse(const string &value) {
  return this->parse(value.data(), value.size());
}
