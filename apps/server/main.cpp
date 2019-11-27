#include "DatasetManager.hpp"
#include "voxer/OSPRayRenderer.hpp"
#include <iostream>
#include <utility>
#include <uwebsockets/App.h>

using namespace std;
using namespace voxer;

struct PerWebSocketData {};

int main(int argc, const char **argv) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " /path/to/database.json" << endl;
    return 0;
  }

  // create data manager
  DatasetManager datasets;
  datasets.load(string(argv[1]));
  UserManager users;
  ConfigManager configs;
  CommandParser parser;

  // run server
  uWS::App::WebSocketBehavior behavior{uWS::SHARED_COMPRESSOR, 1024 * 1024, 10,
                                       1024 * 1024 * 1024};
  behavior.open = [](uWS::WebSocket<false, true> *ws, uWS::HttpRequest *) {
    cout << ws->getRemoteAddress() << " connected " << endl;
  };
  behavior.message = [](uWS::WebSocket<false, true> *ws,
                        std::string_view message, uWS::OpCode opCode) {
    auto command = parser.parse(message.data(), message.size());
    if (command == Command::Render) {
      pipeline.run(parameters);
    }
    ws->send(message, opCode);
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
