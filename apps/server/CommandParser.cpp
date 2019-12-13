#include "CommandParser.hpp"
#include "utils.hpp"
#include <stdexcept>
#include <string>
#include <string_view>
#include <voxer/utils.hpp>

using namespace voxer;
using namespace std;

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
  }

  throw runtime_error("invalid JSON: unknown command type");
}

Command CommandParser::parse(const string &value) {
  return this->parse(value.data(), value.size());
}
