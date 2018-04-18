#define TJE_IMPLEMENTATION
#define PORT 3000
// #include "Dataset.h"
#include "third_party/uWebSockets/uWS.h"
#include "ospray/ospcommon/vec.h"
#include "ospray/ospray_cpp.h"
#include "third_party/rapidjson/document.h"
#include "third_party/tiny_jpeg.h"
#include <cassert>
#include <chrono>
#include <errno.h>
#include <iostream>
#include <string>

using namespace std;

void encode(void *context, void *data, int size) {
  auto buf = (vector<char> *)context;
  auto res = (char *)data;
  for (auto i = 0; i < size; i++) {
    buf->push_back(*(res + i));
  }
}

void writeJPEG(const char *fileName, const int &width, const int &height,
               const uint32_t *pixel) {
  FILE *file = fopen(fileName, "wb");
  if (file == nullptr) {
    fprintf(stderr, "fopen('%s', 'wb') failed: %d", fileName, errno);
    return;
  }

  if (!tje_encode_to_file_at_quality(fileName, 3, width, height, 4,
                                     (const unsigned char *)pixel)) {
    fprintf(stderr, "Could not write JPEG\n");
  }

  fclose(file);
}

int main() {
  // DatasetManager datasetManager();

  uWS::Hub server;
  rapidjson::Document d;
  string response = "Websocket Server has been started!";
  string invalidJSON = "Invalid JSON!";
  // initialize OSPRay; OSPRay parses (and removes) its commandline
  // parameters, e.g. "--osp:debug"
  int argc = 1;
  OSPError init_error = ospInit(&argc, nullptr);
  if (init_error != OSP_NO_ERROR)
    return init_error;

  server.onMessage([&d, &invalidJSON](uWS::WebSocket<uWS::SERVER> *ws,
                                      char *message, size_t length,
                                      uWS::OpCode opCode) {
    // parse json params
    auto start = chrono::steady_clock::now();

    d.Parse(string(message, length).c_str());
    if (!d.IsObject()) {
      ws->send(invalidJSON.data(), invalidJSON.length(), uWS::TEXT);
      return;
    }
    //    rapidjson::Value &s = d["stars"];
    //    cout << s.GetInt() << endl;

    // render
    ospcommon::vec2i imgSize{1024, 768};
    ospcommon::vec3f cam_pos{0.f, 0.f, 0.f};
    ospcommon::vec3f cam_up{0.f, 1.f, 0.f};
    ospcommon::vec3f cam_view{0.1f, 0.f, 1.f};

    // triangle mesh data
    float vertex[] = {-1.0f, -1.0f, 3.0f, 0.f, -1.0f, 1.0f, 3.0f, 0.f,
                      1.0f,  -1.0f, 3.0f, 0.f, 0.1f,  0.1f, 0.3f, 0.f};
    float color[] = {0.9f, 0.5f, 0.5f, 1.0f, 0.8f, 0.8f, 0.8f, 1.0f,
                     0.8f, 0.8f, 0.8f, 1.0f, 0.5f, 0.9f, 0.5f, 1.0f};
    int32_t index[] = {0, 1, 2, 1, 2, 3};

    ospray::cpp::Camera camera("perspective");
    camera.set("aspect", imgSize.x / (float)imgSize.y);
    camera.set("pos", cam_pos);
    camera.set("dir", cam_view);
    camera.set("up", cam_up);
    camera.commit();
    // commit each object to indicate modifications are done

    // create and setup model and mesh
    ospray::cpp::Geometry mesh("triangles");
    ospray::cpp::Data data(4, OSP_FLOAT3A, vertex);
    // OSP_FLOAT3 format is also supported for vertex positions
    data.commit();
    mesh.set("vertex", data);

    data = ospray::cpp::Data(4, OSP_FLOAT4, color);
    data.commit();
    mesh.set("vertex.color", data);

    data = ospray::cpp::Data(2, OSP_INT3, index);
    // OSP_INT4 format is also supported for triangle indices
    data.commit();
    mesh.set("index", data);

    mesh.commit();

    ospray::cpp::Model world;
    world.addGeometry(mesh);
    world.commit();

    // create renderer
    ospray::cpp::Renderer renderer("scivis");

    ospray::cpp::Light light = renderer.newLight("ambient");
    light.commit();
    auto lightHandle = light.handle();
    ospray::cpp::Data lights(1, OSP_LIGHT, &lightHandle);
    lights.commit();

    renderer.set("aoSamples", 1);
    renderer.set("bgColor", 1.0f);
    renderer.set("model", world);
    renderer.set("camera", camera);
    renderer.set("lights", lights);
    renderer.commit();

    ospray::cpp::FrameBuffer framebuffer(
        imgSize, OSP_FB_SRGBA, OSP_FB_COLOR | /*OSP_FB_DEPTH |*/ OSP_FB_ACCUM);
    framebuffer.clear(OSP_FB_COLOR | OSP_FB_ACCUM);

    uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
    // for (int frames = 0; frames < 10; frames++)
    renderer.renderFrame(framebuffer, OSP_FB_COLOR | OSP_FB_ACCUM);

    fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);

    auto step1 = chrono::steady_clock::now();
    cout << "render: " << chrono::duration_cast<chrono::milliseconds>(step1 - start).count()
         << " ms " << endl;

    vector<char> buf;
    buf.reserve(imgSize.x * imgSize.y * 4);
    // writeJPEG("res.jpeg", imgSize.x, imgSize.y, fb);
    tje_encode_with_func(encode, &buf, 1, imgSize.x, imgSize.y, 4,
                         (const unsigned char *)fb);
    framebuffer.unmap(fb);
    ws->send(buf.data(), buf.size(), uWS::BINARY);

    auto step2 = chrono::steady_clock::now();
    cout << "encode: " << chrono::duration_cast<chrono::milliseconds>(step2 - step1).count()
         << " ms " << endl;
  });

  server.onHttpRequest([&](uWS::HttpResponse *res, uWS::HttpRequest req,
                           char *data, size_t length, size_t remainingBytes) {
    res->end(response.data(), response.length());
  });

  if (server.listen(PORT)) {
    cout << "Server starts at port: " << PORT << endl;
    server.run();
  }

  return 0;
}