#include "CommandParser.hpp"
#include "utils.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <voxer/TransferFunction.hpp>
#include <voxer/utils.hpp>

using namespace voxer;
using namespace std;

static auto parse_dataset(simdjson::ParsedJson::Iterator &pjh,
                          DatasetStore &datasets) -> const Dataset * {
  if (!pjh.is_object()) {
    throw JSON_error("params.dataset[i]", "object");
  }

  if (!pjh.move_to_key("type") || !pjh.is_string()) {
    throw JSON_error("params.dataset[i].type", "string");
  }
  const string type = pjh.get_string();
  pjh.up();
  if (type == "dataset") {
    if (!pjh.move_to_key("name") || !pjh.is_string()) {
      throw JSON_error("params.datasets[i].name", "string");
    }
    string name = pjh.get_string();
    pjh.up();

    string variable_name;
    if (pjh.move_to_key("variable")) {
      if (!pjh.is_string()) {
        throw JSON_error("params.datasets[i].name", "string");
      }
      variable_name = pjh.get_string();
      pjh.up();
    }

    uint32_t timestep = 0;
    if (pjh.move_to_key("timestep")) {
      if (!pjh.is_integer()) {
        throw JSON_error("params.datasets[i].dataset.name", "string");
      }
      timestep = pjh.get_integer();
      pjh.up();
    }

    return &(datasets.get(name, variable_name, timestep));
  } else {
    throw runtime_error("params.datasets not upport");
  }
}

static auto parse_scene(simdjson::ParsedJson::Iterator &pjh,
                        DatasetStore &datasets) -> Scene {
  if (!pjh.is_object()) {
    throw JSON_error("params", "object");
  }

  Scene scene{};

  // parse datasets
  if (!pjh.move_to_key("datasets") || !pjh.is_array()) {
    throw JSON_error("params.datasets", "array");
  }
  pjh.down(); // into array
  do {
    scene.datasets.emplace_back(parse_dataset(pjh, datasets));
  } while (pjh.next());
  pjh.up(); // out of array
  pjh.up(); // back to {}

  // parse transfer functions
  if (!pjh.move_to_key("tfcns") || !pjh.is_array()) {
    throw JSON_error("params.tfcns", "array");
  }
  pjh.down(); // into array
  do {
    scene.tfcns.emplace_back(TransferFunction::deserialize(pjh));
  } while (pjh.next());
  pjh.up(); // out of array
  pjh.up(); // back to {}

  // optional keys

  // parse volumes
  if (!pjh.move_to_key("volumes") || !pjh.is_array()) {
    throw JSON_error("params.volumes", "array");
  }
  pjh.down(); // into array
  do {
    scene.volumes.emplace_back(voxer::Volume::deserialize(pjh));
  } while (pjh.next());
  pjh.up(); // out of array
  pjh.up(); // back to {}

  // parse camera
  if (!pjh.move_to_key("camera")) {
    throw JSON_error("params.camera", "required");
  }
  scene.camera = Camera::deserialize(pjh);
  pjh.up(); // back to {}

  // parse isosurfaces
  if (pjh.move_to_key("isosurfaces")) {
    if (!pjh.is_array()) {
      throw JSON_error("params.isosurfaces", "array");
    }
    pjh.down(); // into array
    do {
      scene.isosurfaces.emplace_back(Isosurface::deserialize(pjh));
    } while (pjh.next());
    pjh.up(); // out of array
    pjh.up(); // back to {}
  }

  // parse slices
  if (pjh.move_to_key("slices")) {
    if (!pjh.is_array()) {
      throw JSON_error("params.slices", "array");
    }
    pjh.down(); // into array
    do {
      scene.slices.emplace_back(Slice::deserialize(pjh));
    } while (pjh.next());
    pjh.up(); // out of array
    pjh.up(); // back to {}
  }

  // TODO: parse lights

  return scene;
}

auto CommandParser::parse(const char *value, uint64_t size,
                          DatasetStore &datasets) -> Command {
  if (!pj.allocate_capacity(size)) {
    throw runtime_error("prepare parsing JSON failed");
  }

  const int res = simdjson::json_parse(value, size, pj);
  if (res != 0) {
    cout << "Error parsing: " << simdjson::error_message(res) << endl;
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
    return {Command::Type::Render, parse_scene(pjh, datasets)};
  }

  if (command_type == "query") {
    return {Command::Type::Query, nullptr};
  }

  if (command_type == "save") {
    return {Command::Type::Save,
            make_pair(extract_params(value), parse_scene(pjh, datasets))};
  }

  if (command_type == "run") {
    // TODO
  }

  throw runtime_error("invalid JSON: unknown command type");
}

Command CommandParser::parse(const string &value, DatasetStore &datasets) {
  return this->parse(value.data(), value.size(), datasets);
}
