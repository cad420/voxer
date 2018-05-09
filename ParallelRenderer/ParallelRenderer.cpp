#define TJE_IMPLEMENTATION
#define PORT 3000
#define DEBUG
#include "GenerateSciVis.h"
#include "ospray/ospcommon/box.h"
#include "ospray/ospcommon/vec.h"
#include "ospray/ospray_cpp.h"
#include "Poco/URI.h"
#undef PING
#include "third_party/RawReader/RawReader.h"
#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/istreamwrapper.h"
#include "third_party/rapidjson/ostreamwrapper.h"
#include "third_party/rapidjson/writer.h"
#include "third_party/tiny_jpeg.h"
#include "third_party/uWebSockets/uWS.h"
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <ctime>
#include <utility>
#include <vector>

using namespace std;
using vec3sz = ospcommon::vec_t<size_t, 3>;
typedef map<string, gensv::LoadedVolume> Datasets;

Datasets loadData();
rapidjson::Document loadConfigure();
string generate(rapidjson::Document &, rapidjson::Value &);
vector<char> render(rapidjson::Document &, Datasets &);

void encode(void *context, void *data, int size) {
  auto buf = (vector<char> *)context;
  auto res = (char *)data;
  for (auto i = 0; i < size; i++) {
    buf->push_back(*(res + i));
  }
}

size_t sizeForDtype(const string &dtype) {
  if (dtype == "uchar" || dtype == "char") {
    return 1;
  }
  if (dtype == "float") {
    return 4;
  }
  if (dtype == "double") {
    return 8;
  }
  return 0;
}

string genID(const int len) {
  string res;
  static const char alphanum[] = "0123456789"
                                 "abcdefghijklmnopqrstuvwxyz";
  for (auto i = 0; i < len; ++i) {
    res.push_back(alphanum[rand() % (sizeof(alphanum) - 1)]);
  }
  return res;
}

int main(int argc, const char **argv) {
  OSPError init_error = ospInit(&argc, argv);
  if (init_error != OSP_NO_ERROR) {
    cerr << "OSP Error" << endl;
    return init_error;
  }

  srand((unsigned)time(0));

  uWS::Hub server;
  rapidjson::Document d;
  auto datasets = loadData();
  auto configures = loadConfigure();
  server.onMessage([&d, &datasets, &server, &configures](uWS::WebSocket<uWS::SERVER> *ws,
                                                char *message, size_t length,
                                                uWS::OpCode opCode) {
    d.Parse(string(message, length).c_str());
    if (!d.IsObject()) {
      ws->send("Invalid JSON!");
      return;
    }
    if (!d.HasMember("operation") || !d["operation"].IsString()) {
      ws->send("invalid operation!");
      return;
    }

    try {
      auto operation = string(d["operation"].GetString());
      if (operation == "generate") {
        auto configID = generate(configures, d["params"]);
        ws->send(configID.c_str());
      } else if (operation == "render") {
        auto buffer = render(d, datasets);
        ws->send(buffer.data(), buffer.size(), uWS::BINARY);
      }
    } catch (string exception) {
      cout << "Exception" << endl;
      cout << exception << endl;
      ws->send(exception.c_str());
    }
  }); // end of lambda

  server.onHttpRequest([&](uWS::HttpResponse *res, uWS::HttpRequest req,
                           char *data, size_t length, size_t remainingBytes) {
    Poco::URI uri(req.getUrl().toString());
    vector<string> segments;
    uri.getPathSegments(segments);
    if (segments.size() == 0) {
      cout << "Root" << endl;
      res->end("Websocket Server has been started!");
    } else {
      auto id = segments[segments.size() - 1];
      if (!configures.HasMember(id.c_str())) {
        cout << "Not find" << endl;
        res->end("404");
        return;
      }
      auto params = uri.getQueryParameters();
      cout << "Params: " << endl;
      for (auto &param : params) {
        cout << param.first << " " << param.second << endl;
      }
      res->end("200");
    }    
  });

  if (server.listen(PORT)) {
    cout << "Server starts at port: " << PORT << endl;
    server.run();
  }

  return 0;
}

Datasets loadData() {
  rapidjson::Document d;
  ifstream filestream;
  filestream.open("datasets.json");
  if (!filestream.is_open()) {
    cerr << "Unable to open file datasets.json!" << endl;
    exit(1);
  }
  rapidjson::IStreamWrapper isw(filestream);
  d.ParseStream(isw);
  if (!d.IsArray()) {
    cerr << "Invalid data file!" << endl;
    exit(1);
  }

  ospcommon::vec3i dimensions;
  Datasets datasets;
  for (auto &info : d.GetArray()) {
    if (!info.IsObject()) {
      cerr << "Invalid data file!" << endl;
      exit(1);
    }
    auto name = info["name"].GetString();
    ospcommon::vec2f valueRange{0, 255};
    if (string(name) == "magnetic") {
      valueRange.x = 0.44;
      valueRange.y = 0.77;
    }
    auto filepath = string(info["path"].GetString());
    auto dimsData = info["dimensions"].GetArray();
    for (auto dim = dimsData.Begin(); dim != dimsData.end(); dim++) {
      dimensions[dim - dimsData.Begin()] = dim->GetInt();
    }
    auto dtype = string(info["dtype"].GetString());
    gensv::RawReader reader(ospcommon::FileName(filepath), vec3sz(dimensions),
                            sizeForDtype(dtype));
    const auto upper = ospcommon::vec3f(dimensions);
    const auto halfLength = dimensions / 2;
    auto volume = gensv::loadVolume(reader, dimensions, dtype, valueRange);
    volume.worldBounds = ospcommon::box3f(ospcommon::vec3f(-halfLength),
                                          ospcommon::vec3f(halfLength));
    volume.bounds.lower -= ospcommon::vec3f(halfLength);
    volume.bounds.upper -= ospcommon::vec3f(halfLength);
    volume.volume.set("gridOrigin",
                      volume.ghostGridOrigin - ospcommon::vec3f(halfLength));
    volume.volume.commit();
    datasets[string(info["name"].GetString())] = volume;
  }
  return datasets;
}

rapidjson::Document loadConfigure() {
  ifstream filestream;
  rapidjson::Document configs;
  filestream.open("configures.json");
  if (!filestream.is_open()) {
    cerr << "Unable to open file configures.json!" << endl;
    exit(1);
  }
  rapidjson::IStreamWrapper isw(filestream);
  configs.ParseStream(isw);
  if (!configs.IsObject()) {
    cerr << "Invalid data file!" << endl;
    exit(1);
  }
  return configs;
}

string generate(rapidjson::Document &configs, rapidjson::Value &params) {
  auto name = genID(10);
  auto rpname = rapidjson::StringRef(name.c_str());
  configs.GetObject().AddMember(rpname, params, configs.GetAllocator());
  ofstream ofs("configures.json");
  rapidjson::OStreamWrapper osw(ofs);
  rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
  configs.Accept(writer);
  return name;
}

vector<char> render(rapidjson::Document &d, Datasets &datasets) {
#ifdef DEBUG
  auto start = chrono::steady_clock::now();
#endif
  auto params = d["params"].GetObject();
  auto datasetData = params["dataset"].GetString();
  auto search = datasets.find(datasetData);
  if (search == datasets.end()) {
    throw string("Volume ") + datasetData + " not found";
  }
  auto volume = search->second;
  ospray::cpp::Model world;
  world.addVolume(volume.volume);
  vector<ospcommon::box3f> regions{volume.bounds};
  ospray::cpp::Data regionData(regions.size() * 2, OSP_FLOAT3, regions.data());
  world.set("regions", regionData);
  world.commit();

  auto &cameraData = params["camera"];
  auto cameraType = cameraData.FindMember("type")->value.GetString();
  auto &cameraPosData = cameraData.FindMember("position")->value;
  auto &cameraUpData = cameraData.FindMember("up")->value;
  auto camGaze = ospcommon::center(volume.worldBounds);
  auto &imageData = params["image"];
  ospcommon::vec2i imgSize{imageData["width"].GetInt(),
                           imageData["height"].GetInt()};
  ospcommon::vec3f camPos{cameraPosData.FindMember("x")->value.GetFloat(),
                          cameraPosData.FindMember("y")->value.GetFloat(),
                          cameraPosData.FindMember("z")->value.GetFloat()};
  ospcommon::vec3f camUp{cameraUpData.FindMember("x")->value.GetFloat(),
                         cameraUpData.FindMember("y")->value.GetFloat(),
                         cameraUpData.FindMember("z")->value.GetFloat()};

  ospray::cpp::Camera camera(cameraType);
  camera.set("aspect", (float)imgSize.x / (float)imgSize.y);
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

  for (int frames = 0; frames < 5; frames++)
    renderer.renderFrame(framebuffer, OSP_FB_COLOR | OSP_FB_ACCUM);

#ifdef DEBUG
  auto step1 = chrono::steady_clock::now();
  cout << "render: "
       << chrono::duration_cast<chrono::milliseconds>(step1 - start).count()
       << " ms " << endl;
#endif

  uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
  vector<char> buf;
  buf.reserve(imgSize.x * imgSize.y * 4);
  tje_encode_with_func(encode, &buf, 1, imgSize.x, imgSize.y, 4,
                       (const unsigned char *)fb);
  framebuffer.unmap(fb);

#ifdef DEBUG
  auto step2 = chrono::steady_clock::now();
  cout << "encode: "
       << chrono::duration_cast<chrono::milliseconds>(step2 - step1).count()
       << " ms " << endl;
#endif
  return buf;
}
