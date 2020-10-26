#include "Service/AnnotationService.hpp"
#include "Service/DatasetService.hpp"
#include "Service/SliceService.hpp"
#include "Service/VolumeRenderingService.hpp"
#include "Store/DatasetStore.hpp"
#include "uWebSockets/App.h"
#include <cxxopts.hpp>
#include <iostream>
#include <utility>
#include <voxer/Data/Image.hpp>

using namespace std;
using namespace voxer::remote;

struct UserData {};

int main(int argc, char **argv) {
  cxxopts::Options options(
      "VoxerRemote", "VoxerRemote: scientific visualization cloud service");
  auto add_option = options.add_options();
  add_option("h,help", "Show usage");
  add_option("p,port", "Port listening",
             cxxopts::value<uint16_t>()->default_value("3040"));
  add_option("d,debug", "Enable debugging");
  add_option("v,verbose", "Verbose output",
             cxxopts::value<bool>()->default_value("false"));

  uint16_t port;
  try {
    auto result = options.parse(argc, argv);

    if (result.count("help") != 0) {
      cout << options.help() << endl;
      return 0;
    }

    port = result["port"].as<uint16_t>();
  } catch (exception &error) {
    cout << error.what() << "\n";
    cout << "Try `VoxerRemote --help` for more information.\n";
    return 1;
  }

  DatasetStore datasets;

  DatasetService dataset_service{};
  dataset_service.m_datasets = &datasets;
  VolumeRenderingService volume_rendering_service{};
  volume_rendering_service.m_datasets = &datasets;
  SliceService slice_service{};
  slice_service.m_datasets = &datasets;
  AnnotationService annotation_service{};
  annotation_service.m_datasets = &datasets;

  vector<AbstractService *> services{&dataset_service, &slice_service,
                                     &volume_rendering_service,
                                     &annotation_service};

  auto app = uWS::App();
  // configure websocket server
  for (auto service : services) {
    auto protocol = service->get_protocol();
    switch (protocol) {
    case AbstractService::Protocol::HTTP: {
      std::string body;
      app.post(service->get_path(), [service, &body](uWS::HttpResponse<false> *res,
                                              uWS::HttpRequest *req) {
        res->onAborted([&body]() {
          body = "";
        });
        res->onData([service, &body](std::string_view data, bool last) {
          body += data;
          if (last) {
            service->on_message(reinterpret_cast<const char *>(body.data()),
                                body.size());
            body = "";
          }
        });
        service->m_send = [res](const uint8_t *data, uint32_t size,
                                bool is_binary) {
          res->end(string_view(reinterpret_cast<const char *>(data), size));
        };
      });
      break;
    }
    case AbstractService::Protocol::WebSocket: {
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
      break;
    }
    }
  }

  // run server
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
