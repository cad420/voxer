#pragma once
#include "DatasetStore.hpp"
#include <functional>
#include <iostream>
#include <simdjson/jsonparser.h>
#include <string>
#include <utility>
#include <variant>
#include <voxer/Scene.hpp>

struct Command {
  enum class Type { Render, Save, Query, RunPipeline };

  Type type = Type::Render;
  std::variant<voxer::Scene,                                      // for render
               std::pair<std::string, voxer::Scene>,              // for save
               std::function<voxer::Scene(const voxer::Scene &)>, // for run
               std::nullptr_t                                     // for query
               >
      params = nullptr;
};

class CommandParser {
public:
  auto parse(const std::string &value, DatasetStore &datasets) -> Command;
  auto parse(const char *value, uint64_t size, DatasetStore &datasets)
      -> Command;

private:
  simdjson::ParsedJson pj;
};
