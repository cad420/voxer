#define TJE_IMPLEMENTATION
#define PORT 3000
#define DEBUG true
#include "GenerateSciVis.h"
#include "ospray/ospcommon/vec.h"
#include "ospray/ospcommon/box.h"
#include "ospray/ospray_cpp.h"
#undef PING
#include "third_party/rapidjson/document.h"
#include "third_party/tiny_jpeg.h"
#include "third_party/uWebSockets/uWS.h"
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

int main(int argc, const char **argv) {
  // DatasetManager datasetManager();

  uWS::Hub server;
  rapidjson::Document d;
  string response = "Websocket Server has been started!";
  OSPError init_error = ospInit(&argc, argv);
  if (init_error != OSP_NO_ERROR)
    return init_error;

  server.onMessage([&d](uWS::WebSocket<uWS::SERVER> *ws, char *message,
                        size_t length, uWS::OpCode opCode) {
#ifdef DEBUG
    auto start = chrono::steady_clock::now();
#endif

    // parse json params
    d.Parse(string(message, length).c_str());
    if (!d.IsObject()) {
      ws->send("Invalid JSON!");
      return;
    }
    if (!d.HasMember("operation") || !d["operation"].IsString()) {
      ws->send("invalid operation!");
      return;
    }

    // render
    auto &datasetData = d["dataset"];
    auto datasetName = datasetData.FindMember("name")->value.GetString();
    auto &datasetDimData = datasetData.FindMember("dimension")->value;
    auto datasetDType = datasetData.FindMember("dtype")->value.GetString();
    auto &datasetValueRangeData = datasetData.FindMember("valueRange")->value;
    ospcommon::vec3i dimensions {
      datasetDimData.FindMember("x")->value.GetInt(),
      datasetDimData.FindMember("y")->value.GetInt(),
      datasetDimData.FindMember("z")->value.GetInt()
    };
    ospcommon::vec2f valueRange {
      datasetValueRangeData.FindMember("begin")->value.GetFloat(),
      datasetValueRangeData.FindMember("end")->value.GetFloat(),
    };
    auto volume = gensv::loadVolume(datasetName, dimensions, datasetDType, valueRange);

    // Translate the volume to center it
    const auto upper = ospcommon::vec3f(dimensions);
    const auto halfLength = dimensions / 2;
    auto worldBounds = ospcommon::box3f(ospcommon::vec3f(-halfLength), ospcommon::vec3f(halfLength));
    volume.bounds.lower -= ospcommon::vec3f(halfLength);
    volume.bounds.upper -= ospcommon::vec3f(halfLength);
    volume.volume.set("gridOrigin", volume.ghostGridOrigin - ospcommon::vec3f(halfLength));
    volume.volume.commit();
    ospray::cpp::Model world;
    world.addVolume(volume.volume);

    vector<ospcommon::box3f> regions{volume.bounds};
    ospray::cpp::Data regionData(regions.size() * 2, OSP_FLOAT3, regions.data());
    world.set("regions", regionData);
    world.commit();

    auto &cameraData = d["camera"];
    auto cameraType = cameraData.FindMember("type")->value.GetString();
    auto &cameraPosData = cameraData.FindMember("position")->value;
    auto &cameraUpData = cameraData.FindMember("up")->value;
    auto camGaze = ospcommon::center(worldBounds);    
    ospcommon::vec2i imgSize{1024, 768};
    ospcommon::vec3f camPos{
      cameraPosData.FindMember("x")->value.GetFloat(),
      cameraPosData.FindMember("y")->value.GetFloat(),
      cameraPosData.FindMember("z")->value.GetFloat()
    };
    ospcommon::vec3f camUp{
      cameraUpData.FindMember("x")->value.GetFloat(),
      cameraUpData.FindMember("y")->value.GetFloat(),
      cameraUpData.FindMember("z")->value.GetFloat()
    };

    ospray::cpp::Camera camera(cameraType);
    camera.set("aspect", imgSize.x / (float)imgSize.y);
    camera.set("pos", camPos);
    camera.set("dir", camGaze - camPos);
    camera.set("up", camUp);
    camera.commit();

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

#ifdef DEBUG
    auto step1 = chrono::steady_clock::now();
    cout << "render: "
         << chrono::duration_cast<chrono::milliseconds>(step1 - start).count()
         << " ms " << endl;
#endif

    vector<char> buf;
    buf.reserve(imgSize.x * imgSize.y * 4);
    tje_encode_with_func(encode, &buf, 1, imgSize.x, imgSize.y, 4,
                         (const unsigned char *)fb);
    framebuffer.unmap(fb);
    ws->send(buf.data(), buf.size(), uWS::BINARY);

#ifdef DEBUG
    auto step2 = chrono::steady_clock::now();
    cout << "encode: "
         << chrono::duration_cast<chrono::milliseconds>(step2 - step1).count()
         << " ms " << endl;
#endif
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

void setValue(ospray::cpp::ManagedObject& target, rapidjson::Document& data, const char *key, const char *type) {
  bool valid = data.IsObject();
  if (!valid) throw "Invalid data";
  rapidjson::Value& value = data[key];

  if (type == "string") valid = value.IsString();
  else if (type == "float") valid = value.IsFloat();
  else if (type == "Int") valid = value.IsInt();

  if (!valid) throw string("Invalid value for ") + string(key);

  if (type == "string") target.set(key, value.GetString());
  else if (type == "float") target.set(key, value.GetFloat());
  else if (type == "int") target.set(key, value.GetInt());
}


ospray::cpp::Camera createCamera(rapidjson::Document& data) {
  ospray::cpp::Camera camera;
  setValue(camera, data, "type", "string");
  setValue(camera, data, "aspect", "string");
  return camera;
}

ospray::cpp::Light createLight(rapidjson::Document& data) {
  // ospray::cpp::Light light;
  // setValue(light, data, "type", "string");
  // setValue(light, data, "aspect", "string");
  // return light;
}
