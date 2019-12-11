#include "CommandParser.hpp"
#include "utils.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <voxer/TransferFunction.hpp>

using namespace voxer;
using namespace std;

static inline auto is_number(const simdjson::ParsedJson::Iterator &pjh)
    -> bool {
  return (pjh.is_double() || pjh.is_integer());
}

static inline auto get_number(const simdjson::ParsedJson::Iterator &pjh)
    -> float {
  if (pjh.is_integer()) {
    return static_cast<float>(pjh.get_integer());
  }
  return static_cast<float>(pjh.get_double());
}

static auto parse_dataset(simdjson::ParsedJson::Iterator &pjh,
                          DatasetStore &datasets) -> const Dataset * {

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

static auto parse_tfcn(simdjson::ParsedJson::Iterator &pjh)
    -> TransferFunction {
  if (!pjh.is_array()) {
    throw JSON_error("params.volumes[i].tfcns[j]", "array");
  }
  pjh.down();

  TransferFunction tfcn{};
  do {
    if (!pjh.is_object()) {
      throw JSON_error("params.volumes[i].tfcns[j][k]", "object");
    }

    if (!pjh.move_to_key("x") || !is_number(pjh)) {
      throw JSON_error("params.volumes[i].tfcns[j][k].x", "double");
    }
    tfcn.stops.emplace_back(get_number(pjh));
    pjh.up();

    if (!pjh.move_to_key("y") || !is_number(pjh)) {
      throw JSON_error("params.volumes[i].tfcns[j][k].y", "double");
    }
    tfcn.opacities.emplace_back(get_number(pjh));
    pjh.up();

    if (!pjh.move_to_key("color") || !pjh.is_string()) {
      throw JSON_error("params.volumes[i].tfcns[j][k].color", "double");
    }
    tfcn.colors.emplace_back(hex_color_to_float(pjh.get_string()));
    pjh.up();
  } while (pjh.next());
  pjh.up();

  return tfcn;
}

static auto parse_volume(simdjson::ParsedJson::Iterator &pjh) -> Volume {
  if (!pjh.is_object()) {
    throw JSON_error("params.volumes[i]", "object");
  }

  Volume volume{};

  if (!pjh.move_to_key("dataset") || !pjh.is_integer()) {
    throw JSON_error("params.volumes[i].dataset", "integer");
  }
  volume.dataset_idx = pjh.get_integer();
  pjh.up();

  if (!pjh.move_to_key("tfcn") || !pjh.is_integer()) {
    throw JSON_error("params.volumes[i].tfcn", "integer");
  }
  volume.tfcn_idx = pjh.get_integer();
  pjh.up();

  if (pjh.move_to_key("spacing")) {
    if (!pjh.is_array()) {
      throw JSON_error("params.volumes[i].spacing", "array");
    }
    for (size_t i = 0; i < 3; i++) {
      if (!pjh.move_to_index(0) || !is_number(pjh)) {
        throw JSON_error("params.volumes[i].spacing[j]", "number");
      }
      volume.spacing[i] = get_number(pjh);
      pjh.up();
    }
    pjh.up();
  }

  if (!pjh.move_to_key("render") || (!pjh.is_false() && !pjh.is_true())) {
    throw JSON_error("params.volumes[i].render", "bool");
  }
  volume.render = pjh.is_true();
  pjh.up();

  return volume;
}

static auto parse_camera(simdjson::ParsedJson::Iterator &pjh) -> Camera {
  Camera camera{};

  // TODO: maybe not perspective

  if (!pjh.move_to_key("width") || !pjh.is_integer()) {
    throw JSON_error("params.camera.width", "integer");
  }
  camera.width = pjh.get_integer();
  pjh.up();

  if (!pjh.move_to_key("height") || !pjh.is_integer()) {
    throw JSON_error("params.camera.height", "integer");
  }
  camera.height = pjh.get_integer();
  pjh.up();

  const array<string, 3> indexes = {"pos", "dir", "up"};
  const array<array<float, 3> *, 3> targets = {&camera.pos, &camera.dir,
                                               &camera.up};
  for (size_t i = 0; i < indexes.size(); i++) {
    string idx = indexes[i];
    if (!pjh.move_to_key(idx.c_str()) || !pjh.is_array()) {
      throw JSON_error("params.camera." + idx, "array");
    }
    for (size_t j = 0; j < 3; j++) {
      if (!pjh.move_to_index(j) || !is_number(pjh)) {
        throw JSON_error("params.camera." + idx + "[i]", "number");
      }
      (*targets[i])[j] = get_number(pjh);
      pjh.up();
    }
    pjh.up();
  }

  return camera;
}

static auto parse_isosurface(simdjson::ParsedJson::Iterator &pjh)
    -> Isosurface {
  if (!pjh.is_object()) {
    throw JSON_error("params.isosurfaces[i]", "object");
  }

  Isosurface isosurface{};

  if (!pjh.move_to_key("value") || !is_number(pjh)) {
    throw JSON_error("params.isosurfaces[i].value", "number");
  }
  isosurface.value = get_number(pjh);
  pjh.up();

  if (!pjh.move_to_key("volume") || !pjh.is_integer()) {
    throw JSON_error("params.isosurfaces[i].volume", "integer");
  }
  isosurface.volume_idx = pjh.get_integer();
  pjh.up();

  return isosurface;
}

static auto parse_slice(simdjson::ParsedJson::Iterator &pjh) -> Slice {
  if (!pjh.is_object()) {
    throw JSON_error("params.slices[i]", "object");
  }

  Slice slice{};

  if (!pjh.move_to_key("coef") || !pjh.is_array()) {
    throw JSON_error("params.slices[i].coef", "array");
  }
  for (size_t i = 0; i < 4; i++) {
    if (!pjh.move_to_index(i) || !is_number(pjh)) {
      throw JSON_error("params.slices[i].coef[j]", "double");
    }
    slice.coef[i] = get_number(pjh);
  }
  pjh.up();

  if (!pjh.move_to_key("volume") || !pjh.is_integer()) {
    throw JSON_error("params.slices[i].volume", "integer");
  }
  slice.volume_idx = pjh.get_integer();
  pjh.up();

  return slice;
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
    scene.tfcns.emplace_back(parse_tfcn(pjh));
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
    scene.volumes.emplace_back(parse_volume(pjh));
  } while (pjh.next());
  pjh.up(); // out of array
  pjh.up(); // back to {}

  // parse camera
  if (!pjh.move_to_key("camera") || !pjh.is_object()) {
    throw JSON_error("params.camera", "object");
  }
  scene.camera = parse_camera(pjh);
  pjh.up(); // back to {}

  // parse isosurfaces
  if (pjh.move_to_key("isosurfaces")) {
    if (!pjh.is_array()) {
      throw JSON_error("params.isosurfaces", "array");
    }
    pjh.down(); // into array
    do {
      scene.isosurfaces.emplace_back(parse_isosurface(pjh));
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
      scene.slices.emplace_back(parse_slice(pjh));
    } while (pjh.next());
    pjh.up(); // out of array
    pjh.up(); // back to {}
  }

  // TODO: parse lights

  return scene;
}

Command CommandParser::parse(const char *value, uint64_t size,
                             DatasetStore &datasets) {
  if (!pj.allocate_capacity(size)) {
    throw runtime_error("prepare parsing JSON failed");
  }

  const int res = simdjson::json_parse(value, size, pj);
  if (res != 0) {
    std::cout << "Error parsing: " << simdjson::error_message(res) << std::endl;
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

  throw runtime_error("invalid JSON: unknown command type");
}

Command CommandParser::parse(const std::string &value, DatasetStore &datasets) {
  return this->parse(value.data(), value.size(), datasets);
}
