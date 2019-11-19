#include <iostream>
#include <uwebsockets/App.h>
#include "voxer/DatasetManager.hpp"
#include "voxer/OSPRayRenderer.hpp"

using namespace std;
using namespace voxer;

struct PerWebSocketData {};

int main(int, const char **) {
    // create data manager
    DatasetManager manager;
    manager.load("");

    // create renderers
    OSPRayRenderer renderer;

    auto server = uWS::App().ws<PerWebSocketData>("/*", {
        .compression = uWS::SHARED_COMPRESSOR,
        .maxPayloadLength = 1024 * 1024,
        .idleTimeout = 10,
        .maxBackpressure = 1024 * 1024 * 1204,
        .open = [](auto *ws, auto *req) {
           cout << "connected " << endl;
         },
         .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
            ws->send(message, opCode);
         },
         .close = [](auto *ws, int code, string_view message) {
           cout << "closed" << endl;
        },
    });

    const auto port = 3000;
    server.listen(3000, [](auto *token) {
       if (!token) {
           cout << " failed to listen on port " << port << endl;
           return;
       }

       cout << "listening on port " << port << endl;
    }).run();

    return 0;
}
