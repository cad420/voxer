#include "CommandParser.hpp"
#include "utils.hpp"
#include <stdexcept>
#include <string>
#include <string_view>
#include <voxer/utils.hpp>

using namespace voxer;
using namespace std;

static auto create_modifier(simdjson::ParsedJson::Iterator pjh)
    -> pair<string, SceneModifier> {
  if (!pjh.is_object()) {
    throw JSON_error("params", "object");
  }

  if (!pjh.move_to_key("id") || !pjh.is_string()) {
    throw JSON_error("params.id", "string");
  }
  string id = pjh.get_string();
  pjh.up();

  auto modifier = [json = move(pjh)](const Scene &origin) mutable -> Scene {
    Scene scene = origin;

    if (json.move_to_key("camera")) {
      if (json.is_object()) {
        if (json.move_to_key("width")) {
          if (json.is_integer()) {
            scene.camera.width = json.get_integer();
          }
        }
        json.up();

        if (json.move_to_key("height")) {
          if (json.is_integer()) {
            scene.camera.height = json.get_integer();
          }
        }
        json.up();

        array<string, 3> keys = {"pos", "dir", "up"};
        array<array<float, 3> *, 3> targets = {
            &(scene.camera.pos), &(scene.camera.dir), &(scene.camera.up)};
        for (size_t i = 0; i < keys.size(); i++) {
          if (json.move_to_key(keys[i].c_str())) {
            if (json.is_array()) {
              for (size_t j = 0; j < 3; j++) {
                if (json.move_to_index(j)) {
                  if (is_number(json)) {
                    (*targets[i])[j] = get_number(json);
                  }
                }
                json.up();
              }
            }
          }
          json.up();
        }
      }
    }
    json.up();

    if (json.move_to_key("tfcns")) {
      if (json.is_array()) {
        json.down(); // into array
        scene.tfcns.clear();
        do {
          scene.tfcns.emplace_back(TransferFunction::deserialize(json));
        } while (json.next());
        json.up(); // out of array
      }
    }
    json.up(); // back to {}

    json.up();

    return scene;
  };

  return make_pair(move(id), move(modifier));
}

auto CommandParser::parse(const char *value, uint64_t size) -> Command {
  if (!pj.allocate_capacity(size)) {
    throw runtime_error("prepare parsing JSON failed");
  }

  const int res = simdjson::json_parse(value, size, pj);
  if (res != 0) {
    throw runtime_error("Error parsing: " + simdjson::error_message(res));
  }

  simdjson::ParsedJson::Iterator pjh(pj);
  if (!pjh.is_ok()) {
    throw runtime_error("invalid json");
  }

  if (!pjh.is_object()) {
    throw JSON_error("root", "object");
  }

  if (!pjh.move_to_key("type") || !pjh.is_string()) {
    throw JSON_error("type", "string");
  }

  string_view command_type = pjh.get_string();
  pjh.up();

  pjh.move_to_key("params");
  if (command_type == "render") {
    return {Command::Type::Render, Scene::deserialize(pjh)};
  }

  if (command_type == "query") {
    if (!pjh.is_object()) {
      throw JSON_error("params", "object");
    }

    if (!pjh.move_to_key("target") || !pjh.is_string()) {
      throw JSON_error("params.target", "string");
    }

    string target = pjh.get_string();
    pjh.up();
    if (target == "datasets") {
      return {Command::Type::QueryDatasets, nullptr};
    }

    if (target == "pipelines") {
      return {Command::Type::QueryPipelines, nullptr};
    }

    if (target == "dataset") {
      if (!pjh.move_to_key("params") || !pjh.is_object()) {
        throw JSON_error("params.params", "object");
      }
      return {Command::Type::QueryDataset, SceneDataset::deserialize(pjh)};
    }

    if (target == "pipeline") {
      if (!pjh.move_to_key("id") || !pjh.is_string()) {
        throw JSON_error("params.id", "string when params.target == pipeline");
      }
      return {Command::Type::QueryPipeline, pjh.get_string()};
    }
  }

  if (command_type == "save") {
    return {Command::Type::Save,
            make_pair(extract_params(value), Scene::deserialize(pjh))};
  }

  if (command_type == "run") {
    return {Command::Type::RunPipeline, create_modifier(move(pjh))};
  }

  throw runtime_error("invalid JSON: unknown command type");
}

Command CommandParser::parse(const string &value) {
  return this->parse(value.data(), value.size());
}
