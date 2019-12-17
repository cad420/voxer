#pragma once
#include <random>
#include <regex>
#include <string>
#include <vector>

inline auto nanoid(uint8_t size = 16) -> std::string {
  static const char *url =
      "ModuleSymbhasOwnPr-0123456789ABCDEFGHNRVfgctiUvz_KqYTJkLxpZXIjQW";
  std::string id;

  std::random_device engine;
  for (size_t i = 0; i < size; i++) {
    unsigned byte = engine();
    id += url[byte & 63u];
  }

  return id;
}

inline auto extract_params(const std::string &json) -> std::string {
  std::regex params_regex(R"("params":\s*(\{[^]+\})[\s|\n]*\})",
                          std::regex_constants::ECMAScript);
  std::smatch match;
  std::regex_search(json, match, params_regex);
  if (match.empty()) {
    return "";
  }

  return match[1];
}

inline auto histogram_to_json(const std::vector<uint32_t> &data)
    -> std::string {
  std::string res = "[";
  for (auto &value : data) {
    res += std::to_string(value) + ",";
  }
  res[res.find_last_of(',')] = ']';
  return res;
}
