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

    if (json.move_to_key("camera") && json.is_object()) {
      if (json.move_to_key("width") && json.is_integer()) {
        scene.camera.width = json.get_integer();
      }
      if (json.move_to_key("height") && json.is_integer()) {
        scene.camera.height = json.get_integer();
      }
      array<const char *, 3> keys = {"pos", "dir", "up"};
      array<array<float, 3> *, 3> targets = {
          &(scene.camera.pos), &(scene.camera.dir), &(scene.camera.up)};
      for (size_t i = 0; i < keys.size(); i++) {
        if (json.move_to_key(keys[i]) && json.is_array()) {
          for (size_t j = 0; j < 3; j++) {
            if (json.move_to_index(j) && is_number(json)) {
              (*targets[i])[j] = get_number(json);
              json.up();
            }
          }
          json.up();
        }
      }
      json.up();
    }

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
    return {Command::Type::Query, nullptr};
  }

  if (command_type == "save") {
    return {Command::Type::Save,
            make_pair(extract_params(value), Scene::deserialize(pjh))};
  }

  if (command_type == "run") {
    // TODO

    return {Command::Type::RunPipeline, create_modifier(move(pjh))};
  }

  throw runtime_error("invalid JSON: unknown command type");
}

Command CommandParser::parse(const string &value) {
  return this->parse(value.data(), value.size());
}
