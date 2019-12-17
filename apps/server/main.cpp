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

using namespace std;
using namespace voxer;

struct UserData {};

int main(int argc, const char **argv) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " </path/to/database.json>" << endl;
    return 0;
  }

  // prepare datasets
  DatasetStore datasets;
  datasets.load_from_file(string(argv[1]));

  // load exist pipelines
  PipelineStore pipelines;
  if (argc >= 3) {
    // TODO: load ALL exist pipelines
    pipelines.load_from_file(argv[3]);
  }

  CommandParser parser{};
  OSPRayRenderer renderer{datasets};

  // configure server
  uWS::App::WebSocketBehavior behavior{uWS::SHARED_COMPRESSOR, 1024 * 1024,
                                       30 * 60, 1024 * 1024 * 1024};

  behavior.open = [](uWS::WebSocket<false, true> *ws, uWS::HttpRequest *) {
    cout << "connected " << endl;
  };

  behavior.message = [&datasets, &parser, &pipelines,
                      &renderer](uWS::WebSocket<false, true> *ws,
                                 std::string_view message, uWS::OpCode) {
    assert(ws->getUserData() != nullptr);
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
      case Command::Type::Query: {
        ws->send(datasets.print(), uWS::OpCode::TEXT);
        break;
      }
      case Command::Type::QueryDataset: {
        auto scene_dataset = get<SceneDataset>(command.params);
        auto &dataset = datasets.get(scene_dataset);
        ws->send(histogram_to_json(dataset.histogram), uWS::OpCode::TEXT);
        break;
      }
      case Command::Type::RunPipeline: {
        auto save = get<pair<string, SceneModifier>>(command.params);

        // merge params changes
        auto &origin = pipelines.get(save.first);
        auto scene = save.second(origin);
        auto image = renderer.render(scene);
        auto compressed = Image::encode(image, Image::Format::JPEG);
        auto size = compressed.data.size();
        ws->send(
            string_view(reinterpret_cast<char *>(compressed.data.data()), size),
            uWS::OpCode::BINARY);
        break;
      }
      case Command::Type::Save: {
        auto save = get<pair<string, Scene>>(command.params);
        auto id = pipelines.save(save.first, move(save.second));
        ws->send(fmt::format(R"({{"command":"save","value": "{}"}})", id));
        break;
      }
      default:
        break;
      }
    } catch (const exception &excpetion) {
      ws->send(fmt::format(R"({{"error": "{}"}})", excpetion.what()),
               uWS::OpCode::TEXT);
    }
  };

  behavior.close = [](uWS::WebSocket<false, true> *ws, int, string_view) {
    cout << ws->getRemoteAddress() << "closed" << endl;
  };

  // run server
  const auto port = 3000;
  auto on_success = [port](auto *token) {
    if (!token) {
      cout << " failed to listen on port " << port << endl;
      return;
    }
    cout << "listening on port " << port << endl;
  };
  auto server = uWS::App().ws<UserData>("/*", move(behavior));
  server.listen(port, on_success).run();

  return 0;
}
