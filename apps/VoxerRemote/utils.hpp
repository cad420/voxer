#pragma once
#include <random>
#include <regex>
#include <string>
#include <vector>

struct NoCopy {
  NoCopy() = default;
  NoCopy(const NoCopy &) = delete;
  auto operator=(const NoCopy &) -> NoCopy & = delete;
};

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
