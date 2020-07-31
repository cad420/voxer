#include "CommandParser.hpp"
#include "service/DatasetService.hpp"
#include "service/SliceService.hpp"
#include "service/VolumeRenderingService.hpp"
#include "uWebSockets/App.h"
#include <iostream>
#include <utility>
#include <voxer/DatasetStore.hpp>
#include <voxer/Image.hpp>

using namespace std;
using namespace voxer;

struct UserData {};

int main(int argc, const char **argv) {
  DatasetStore datasets;

  DatasetService dataset_service{};
  dataset_service.m_datasets = &datasets;
  VolumeRenderingService volume_rendering_service{};
  volume_rendering_service.m_datasets = &datasets;
  SliceService slice_service{};
  slice_service.m_datasets = &datasets;

  vector<Service *> services{&dataset_service, &volume_rendering_service,
                             &slice_service};

  auto app = uWS::App();

  // configure websocket server
  for (auto service : services) {
    uWS::App::WebSocketBehavior behavior{uWS::SHARED_COMPRESSOR, 1024 * 1024,
                                         30 * 60, 1024 * 1024 * 1024};
    behavior.message = [service](uWS::WebSocket<false, true> *ws,
                                 std::string_view message, uWS::OpCode) {
      service->m_send = [ws](const uint8_t *data, uint32_t size,
                             bool is_binary) {
        ws->send(string_view(reinterpret_cast<const char *>(data), size),
                 is_binary ? uWS::OpCode::BINARY : uWS::OpCode::TEXT);
      };
      service->on_message(message.data(), message.size());
    };
    app.ws<UserData>(service->get_path(), move(behavior));
  }

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
