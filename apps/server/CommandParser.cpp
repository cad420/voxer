#include "CommandParser.hpp"
#include "utils.hpp"
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <voxer/TransferFunction.hpp>

using namespace voxer;
using namespace std;

static auto color_hex_to_floats(const string &str) -> std::array<float, 3> {}

static auto parse_scene(simdjson::ParsedJson::Iterator &pjh) -> Scene {
  if (!pjh.is_object()) {
    throw JSON_error("params", "object");
  }

  pjh.down();
  if (!pjh.move_to_key("volumes") || !pjh.is_array()) {
    throw JSON_error("params.volumes", "array");
  }

  pjh.down();
  do {
    if (!pjh.is_object()) {
      throw JSON_error("params.volumes[i]", "object");
    }

    pjh.move_to_key("tfcn");
    if (!pjh.is_array()) {
      throw JSON_error("params.volumes[i].tfcn", "array");
    }
    pjh.down();

    TransferFunction tfcn;
    do {
      if (!pjh.is_object()) {
        throw JSON_error("params.volumes[i].tfcn[i]", "object");
      }

      ControlPoint point{};
      pjh.move_to_key("x");
      point.stop = static_cast<float>(pjh.get_double());
      pjh.up();
      pjh.move_to_key("y");
      point.opacity = static_cast<float>(pjh.get_double());
      pjh.up();
      point.color = color_hex_to_floats(pjh.get_string());
      pjh.up();
      tfcn.points.emplace_back(point);
    } while (pjh.next());

    pjh.up();
    pjh.move_to_key("dataset");
    if (!pjh.is_object()) {
      throw JSON_error("params.volumes[i].tfcn[i]", "object");
    }
  } while (pjh.next());

  if (!pjh.move_to_key("renderer") || !pjh.is_object()) {
    throw JSON_error("params.renderer", "object");
  }

  return Scene{};
}

Command CommandParser::parse(const char *value, uint64_t size) {
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

  pjh.down();
  if (!pjh.move_to_key("type")) {
    throw JSON_error("type", "string");
  }

  string_view command_type = pjh.get_string();
  pjh.up();
  pjh.move_to_key("params");
  if (command_type == "render") {
    return {Command::Type::Render, parse_scene(pjh)};
  }

  if (command_type == "query") {
    return {Command::Type::Query, nullptr};
  }

  throw runtime_error("invalid JSON: unknown command type");
}

Command CommandParser::parse(const std::string &value) {
  return this->parse(value.data(), value.size());
}
