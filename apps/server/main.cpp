#include "CommandParser.hpp"
#include "DatasetManager.hpp"
#include "voxer/OSPRayRenderer.hpp"
#include <iostream>
#include <utility>
#include <uwebsockets/App.h>
#include <voxer/Image.hpp>

using namespace std;
using namespace voxer;

struct PerWebSocketData {
  CommandParser parser;
  OSPRayRenderer renderer;
};

int main(int argc, const char **argv) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " /path/to/database.json" << endl;
    return 0;
  }

  // create data manager
  DatasetManager datasets;
  datasets.load(string(argv[1]));

  // run server
  uWS::App::WebSocketBehavior behavior{uWS::SHARED_COMPRESSOR, 1024 * 1024, 10,
                                       1024 * 1024 * 1024};

  behavior.open = [](uWS::WebSocket<false, true> *ws, uWS::HttpRequest *) {
    cout << ws->getRemoteAddress() << " connected " << endl;
  };

  behavior.message = [&datasets](uWS::WebSocket<false, true> *ws,
                                 std::string_view message, uWS::OpCode) {
    auto &data = *reinterpret_cast<PerWebSocketData *>(ws->getUserData());
    Command command;
    try {
      command = data.parser.parse(message.data(), message.size());
    } catch (const exception &excpetion) {
      ws->send(excpetion.what());
      return;
    }

    if (command.type == Command::Type::Render) {
      const auto &scene = get<Scene>(command.params);
      auto image = data.renderer.render(scene);
      auto compressed_img = Image::encode(image, Image::Format::JPEG);

    } else if (command.type == Command::Type::Query) {
      ws->send(datasets.print(), uWS::OpCode::TEXT);
    }
  };

  behavior.close = [](uWS::WebSocket<false, true> *ws, int, string_view) {
    cout << ws->getRemoteAddress() << "closed" << endl;
  };

  auto server = uWS::App().ws<PerWebSocketData>("/*", move(behavior));

  const auto port = 3000;
  server
      .listen(3000,
              [](auto *token) {
                if (!token) {
                  cout << " failed to listen on port " << port << endl;
                  return;
                }

                cout << "listening on port " << port << endl;
              })
      .run();

  return 0;
}
