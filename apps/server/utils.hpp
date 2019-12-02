#pragma once
#include <fmt/core.h>
#include <fmt/format.h>
#include <stdexcept>
#include <string_view>

class JSON_error : public std::runtime_error {
public:
  JSON_error(const std::string &key, const std::string &excepted)
      : std::runtime_error("invalid JSON: " + key + " should be " + excepted +
                           ".") {}
};

namespace fmt {
template <> struct formatter<voxer::DatasetVariable> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const voxer::DatasetVariable &variable, FormatContext &ctx) {
    return format_to(ctx.out(), R"({{"name": "{}","timesteps":{}}})",
                     variable.name.c_str(), variable.timesteps.size());
  }
};

template <> struct formatter<voxer::Dataset> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const voxer::Dataset &dataset, FormatContext &ctx) {
    std::string variables = "[";
    for (const auto &it : dataset.variables) {
      const auto &variable = it.second;
      variables += fmt::to_string(variable);
      variables += ",";
    }
    variables[variables.find_last_of(',')] = ']';
    return format_to(
        ctx.out(),
        R"({{"name": "{}","type":"{}","dimensions":[{}],"variables":{}}})",
        dataset.name, dataset.type, fmt::join(dataset.dimensions, ","),
        variables);
  }
};
} // namespace fmt
