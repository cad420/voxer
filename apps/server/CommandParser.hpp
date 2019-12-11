#pragma once
#include "DatasetStore.hpp"
#include <iostream>
#include <simdjson/jsonparser.h>
#include <string>
#include <variant>
#include <voxer/Scene.hpp>

struct Command {
  enum class Type { Render, Save, Query, RunPipeline };

  Type type = Type::Render;
  std::variant<voxer::Scene, std::nullptr_t> params = nullptr;
};

class CommandParser {
public:
  auto parse(const std::string &value, DatasetStore &datasets) -> Command;
  auto parse(const char *value, uint64_t size, DatasetStore &datasets)
      -> Command;

private:
  simdjson::ParsedJson pj;
};
