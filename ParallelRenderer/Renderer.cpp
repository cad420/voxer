#include "Renderer.h"
#include "ospray/ospray_cpp.h"

using namespace std;

vector<unsigned char> Renderer::render(rapidjson::Value &values, gensv::LoadedVolume &volume, map<string, string> *extraParams) {
  auto start = chrono::steady_clock::now();
  auto params = values.GetObject();
  auto datasetData = params["dataset"].GetString();
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
  if (extraParams) {
    auto params = *extraParams;
    if (params.find("width") != params.end()) {
      imgSize.x = stoi(params["width"]);
    }
    if (params.find("height") != params.end()) {
      imgSize.y = stoi(params["height"]);
    }
    if (params.find("pos.x") != params.end()) {
      camPos.x = stoi(params["pos.x"]);
    }
    if (params.find("pos.y") != params.end()) {
      camPos.y = stoi(params["pos.y"]);
    }
    if (params.find("pos.z") != params.end()) {
      camPos.z = stoi(params["pos.z"]);
    }
  }
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

  cout << "render: "
       << chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start).count()
       << " ms " << endl;

  unsigned char *fb = (unsigned char *)framebuffer.map(OSP_FB_COLOR);
  vector<unsigned char> buf(fb, fb + imgSize.x * imgSize.y * 4);
  framebuffer.unmap(fb);

  return buf;
}