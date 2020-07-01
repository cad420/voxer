#include "CommandParser.hpp"
#include "PipelineStore.hpp"
#include "uWebSockets/App.h"
#include <fmt/format.h>
#include <iostream>
#include <utility>
#include <voxer/DatasetStore.hpp>
#include <voxer/Image.hpp>
#include <voxer/RenderingContext.hpp>

using namespace std;
using namespace voxer;

struct UserData {};

int main(int argc, const char **argv) {
  DatasetStore datasets;
  PipelineStore pipelines;

  CommandParser parser{};
  RenderingContext ogl_renderer(RenderingContext::Type::OpenGL);
  RenderingContext ospray_renderer(RenderingContext::Type::OSPRay);

  auto app = uWS::App();

  // configure websocket server
  uWS::App::WebSocketBehavior behavior{uWS::SHARED_COMPRESSOR, 1024 * 1024,
                                       30 * 60, 1024 * 1024 * 1024};

  behavior.message = [&datasets, &parser, &pipelines, &ogl_renderer,
                      &ospray_renderer](uWS::WebSocket<false, true> *ws,
                                        std::string_view message, uWS::OpCode) {
    Command command;
    try {
      command = parser.parse(message.data(), message.size());

      auto &renderer =
          command.engine == EngineType::OSPRay ? ospray_renderer : ogl_renderer;

      switch (command.type) {
      case Command::Type::Render: {
        const auto &scene = get<Scene>(command.params);
        renderer.render(scene, datasets);
        auto &image = renderer.get_colors();
        auto compressed = Image::encode(image, Image::Format::JPEG);
        auto size = compressed.data.size();
        ws->send(
            string_view(reinterpret_cast<char *>(compressed.data.data()), size),
            uWS::OpCode::BINARY);
        break;
      }
      case Command::Type::QueryDatasets: {
        auto msg =
            fmt::format(R"({{"type":"query","target":"datasets","data":{}}})",
                        datasets.print());
        ws->send(msg, uWS::OpCode::TEXT);
        break;
      }
      case Command::Type::QueryDataset: {
        auto scene_dataset = get<SceneDataset>(command.params);
        // TODO: what about differed dataset?
        auto &dataset = datasets.get(scene_dataset.name, scene_dataset.variable,
                                     scene_dataset.timestep);
        auto msg =
            fmt::format(R"({{"type":"query","target":"dataset","data":{}}})",
                        dataset.serialize());
        ws->send(msg, uWS::OpCode::TEXT);
        break;
      }
      case Command::Type::QueryPipelines: {
        auto msg =
            fmt::format(R"({{"type":"query","target":"pipelines","data":{}}})",
                        pipelines.print());
        ws->send(msg, uWS::OpCode::TEXT);
        break;
      }
      case Command::Type ::QueryPipeline: {
        auto &id = get<string>(command.params);
        auto &json = pipelines.get_serialized(id);
        auto msg = fmt::format(
            R"({{"type":"query","target":"pipeline","data":{}}})", json);
        ws->send(msg, uWS::OpCode::TEXT);
        break;
      }
      case Command::Type::RunPipeline: {
        auto save = get<pair<string, SceneModifier>>(command.params);

        // merge params changes
        auto &origin = pipelines.get(save.first);
        auto &modify_scene = save.second;
        auto scene = modify_scene(origin);
        renderer.render(scene, datasets);
        auto &image = renderer.get_colors();
        auto compressed = Image::encode(image, Image::Format::JPEG);
        ws->send(string_view(reinterpret_cast<char *>(compressed.data.data()),
                             compressed.data.size()),
                 uWS::OpCode::BINARY);
        break;
      }
      case Command::Type::Save: {
        auto save = get<pair<string, Scene>>(command.params);
        auto id = pipelines.save(save.first, move(save.second));
        ws->send(fmt::format(R"({{"type":"save","value":"{}"}})", id),
                 uWS::OpCode::TEXT);
        break;
      }
      case Command::Type::AddDataset: {
        auto json = get<string>(command.params);
        datasets.add_from_json(json.c_str(), json.size());
        ws->send(R"({"type":"add"})", uWS::OpCode::TEXT);
        break;
      }
      case Command::Type ::ModifyDataset: {
        auto params = get<pair<string, Scene>>(command.params);
        auto id = params.first;
        auto scene = move(params.second);
        pipelines.update(id, scene);
        ws->send(R"({"type":"modify"})", uWS::OpCode::TEXT);
        break;
      }
      default:
        ws->send(fmt::format(R"({{"error":"unsupported command"}})"));
      }
    } catch (const exception &excpetion) {
      ws->send(fmt::format(R"({{"error": "{}"}})", excpetion.what()),
               uWS::OpCode::TEXT);
    }
  };

  app.ws<UserData>("/render", move(behavior));

  app.ws<UserData>(
      "/datasets",
      {.message = [&datasets](uWS::WebSocket<false, true> *ws,
                              std::string_view message, uWS::OpCode opCode) {
        datasets.load_from_json(message.data(), message.size());
        auto &items = datasets.get();
        for (auto &item : items) {
          ws->send(item.serialize(), uWS::OpCode::TEXT);
        }
      }});

  // run server
  const auto port = 3000;
  auto on_success = [port](auto *token) {
    if (!token) {
      cout << " failed to listen on port " << port << endl;
      return;
    }
    cout << "listening on port " << port << endl;
  };
  app.listen(port, on_success).run();

  return 0;
}
