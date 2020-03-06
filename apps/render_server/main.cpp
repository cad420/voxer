#include "CommandParser.hpp"
#include "PipelineStore.hpp"
#include "utils.hpp"
#include <fmt/format.h>
#include <iostream>
#include <utility>
#include <uwebsockets/App.h>
#include <voxer/DatasetStore.hpp>
#include <voxer/Image.hpp>
#include <voxer/OSPRayRenderer.hpp>
#include <voxer/filter/histogram.hpp>

using namespace std;
using namespace voxer;

struct UserData {};

int main(int argc, const char **argv) {
  // prepare datasets
  DatasetStore datasets;

  if (argc >= 2) {
    datasets.load_from_file(string(argv[1]));
  } else {
    datasets.load();
  }

  // prepare histograms
  map<string, vector<uint32_t>> histograms;
  for (auto &dataset : datasets.get()) {
    histograms.emplace(dataset.id, calculate_histogram(dataset));
  }

  // load exist pipelines
  PipelineStore pipelines;
  if (argc >= 3) {
    pipelines.load_from_directory(argv[2]);
  } else {
    pipelines.load();
  }

  CommandParser parser{};
  OSPRayRenderer renderer{datasets};

  auto app = uWS::App();

  // TODO: configure http routes

  // configure websocket server
  uWS::App::WebSocketBehavior behavior{uWS::SHARED_COMPRESSOR, 1024 * 1024,
                                       30 * 60, 1024 * 1024 * 1024};

  behavior.open = [](uWS::WebSocket<false, true> *ws, uWS::HttpRequest *) {
    cout << "connected " << endl;
  };

  behavior.message = [&datasets, &parser, &pipelines, &histograms,
                      &renderer](uWS::WebSocket<false, true> *ws,
                                 std::string_view message, uWS::OpCode) {
    Command command;
    try {
      command = parser.parse(message.data(), message.size());

      switch (command.type) {
      case Command::Type::Render: {
        const auto &scene = get<Scene>(command.params);
        auto image = renderer.render(scene);
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
        auto item = histograms.find(dataset.id);
        if (item == histograms.end()) {
          histograms.emplace(dataset.id, calculate_histogram(dataset));
          item = histograms.find(dataset.id);
        }
        auto msg =
            fmt::format(R"({{"type":"query","target":"dataset","data":{}}})",
                        histogram_to_json(item->second));
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
        auto image = renderer.render(scene);
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

  behavior.close = [](uWS::WebSocket<false, true> *ws, int, string_view) {
    cout << ws->getRemoteAddress() << "closed" << endl;
  };

  app.ws<UserData>("/*", move(behavior));
  app.post("/", [](uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
    req->
  });

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
