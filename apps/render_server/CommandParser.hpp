#pragma once
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <variant>
#include <voxer/Scene.hpp>
#include <voxer/RenderingContext.hpp>

using SceneModifier = std::function<voxer::Scene(const voxer::Scene &)>;
using EngineType = voxer::RenderingContext::Type;

struct Command {
  enum class Type {
    Render,
    Save,
    QueryDatasets,
    QueryDataset,
    QueryPipelines,
    QueryPipeline,
    RunPipeline,
    AddDataset,
    ModifyDataset
  };

  Type type = Type::Render;
  EngineType engine = EngineType::OSPRay;
  std::variant<voxer::Scene,                          // for render
               std::pair<std::string, voxer::Scene>,  // for save
               std::pair<std::string, SceneModifier>, // for run
               voxer::SceneDataset, // for query specific dataset
               std::nullptr_t,      // for query dataset list or pipeline list
               std::string // for query specific pipeline or add dataset
               >
      params = nullptr;
};

class CommandParser {
public:
  auto parse(const std::string &value) -> Command;
  auto parse(const char *value, uint64_t size) -> Command;

private:
  rapidjson::Document document;
};
