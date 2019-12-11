#include "CommandParser.hpp"
#include "DatasetStore.hpp"
#include <fmt/format.h>
#include <iostream>
#include <utility>
#include <uwebsockets/App.h>
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

  // create data manager
  DatasetStore datasets;
  datasets.load_from_file(string(argv[1]));

  // run server
  CommandParser parser;
  OSPRayRenderer renderer;

  uWS::App::WebSocketBehavior behavior{uWS::SHARED_COMPRESSOR, 1024 * 1024,
                                       30 * 60, 1024 * 1024 * 1024};

  behavior.open = [](uWS::WebSocket<false, true> *ws, uWS::HttpRequest *) {
    cout << "connected " << endl;
  };

  behavior.message = [&datasets, &parser,
                      &renderer](uWS::WebSocket<false, true> *ws,
                                 std::string_view message, uWS::OpCode) {
    assert(ws->getUserData() != nullptr);
    Command command;
    try {
      command = parser.parse(message.data(), message.size(), datasets);
    } catch (const exception &excpetion) {
      ws->send(fmt::format(R"({{"error": "{}"}})", excpetion.what()),
               uWS::OpCode::TEXT);
      return;
    }

    if (command.type == Command::Type::Render) {
      const auto &scene = get<Scene>(command.params);
      auto image = renderer.render(scene);
      auto compressed = Image::encode(image, Image::Format::JPEG);
      auto size = compressed.data.size();
      ws->send(
          string_view(reinterpret_cast<char *>(compressed.data.data()), size),
          uWS::OpCode::BINARY);
    } else if (command.type == Command::Type::Query) {
      ws->send(datasets.print(), uWS::OpCode::TEXT);
    }
  };

  behavior.close = [](uWS::WebSocket<false, true> *ws, int, string_view) {
    cout << ws->getRemoteAddress() << "closed" << endl;
  };

  auto server = uWS::App().ws<UserData>("/*", move(behavior));

  const auto port = 3000;
  auto handler = [port](auto *token) {
    if (!token) {
      cout << " failed to listen on port " << port << endl;
      return;
    }

    cout << "listening on port " << port << endl;
  };
  server.listen(port, handler).run();

  return 0;
}
