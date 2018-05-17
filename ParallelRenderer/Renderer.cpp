#include "Renderer.h"
#include "ospray/ospray_cpp.h"
#include <cstdlib>

using namespace std;
using namespace ospcommon;

vector<unsigned char> Renderer::render(rapidjson::Value &values,
                                        gensv::LoadedVolume &volume, 
                                       map<string, string> *extraParams) {
  auto start = chrono::steady_clock::now();
  auto params = values.GetObject();

  vec2f valueRange{0, 255};
  if (params["dataset"] == "magnetic") {
    valueRange.x = 0.44;
    valueRange.y = 0.77;
  }
  vector<vec3f> colors;
  vector<float> opacities;
  const auto &tfcn = params["tfcn"];
  const auto &colorJSON = tfcn["colors"].GetArray();
  const auto &opacityJSON = tfcn["opacities"].GetArray();
  for (auto &color : colorJSON) {
    const auto hex = string(color.GetString());
    colors.push_back(vec3f{strtol(hex.substr(1, 2).c_str(), nullptr, 16) * 1.0f / 255,
                           strtol(hex.substr(3, 2).c_str(), nullptr, 16) * 1.0f / 255,
                           strtol(hex.substr(5, 2).c_str(), nullptr, 16) * 1.0f / 255});
  }
  for (auto &opacity : opacityJSON) {
    opacities.push_back(opacity.GetFloat());
  }

  ospray::cpp::Data colorsData(colors.size(), OSP_FLOAT3, colors.data());
  ospray::cpp::Data opacityData(opacities.size(), OSP_FLOAT, opacities.data());
  colorsData.commit();
  opacityData.commit();

  volume.tfcn.set("valueRange", valueRange);
  volume.tfcn.set("colors", colorsData);
  volume.tfcn.set("opacities", opacityData);
  volume.tfcn.commit();
  volume.volume.set("voxelRange", valueRange);
  volume.volume.commit();

  ospray::cpp::Model world;
  world.addVolume(volume.volume);
  vector<box3f> regions{volume.bounds};
  ospray::cpp::Data regionData(regions.size() * 2, OSP_FLOAT3, regions.data());
  world.set("regions", regionData);
  world.commit();

  auto &cameraData = params["camera"];
  auto cameraType = cameraData.FindMember("type")->value.GetString();
  auto &cameraPosData = cameraData.FindMember("pos")->value;
  auto &cameraUpData = cameraData.FindMember("up")->value;
  auto &cameraDirData = cameraData.FindMember("dir")->value;
  auto &imageData = params["image"];
  vec2i imgSize{imageData["width"].GetInt(), imageData["height"].GetInt()};
  vec3f camPos{cameraPosData.FindMember("x")->value.GetFloat(),
               cameraPosData.FindMember("y")->value.GetFloat(),
               cameraPosData.FindMember("z")->value.GetFloat()};
  vec3f camUp{cameraUpData.FindMember("x")->value.GetFloat(),
              cameraUpData.FindMember("y")->value.GetFloat(),
              cameraUpData.FindMember("z")->value.GetFloat()};
  vec3f camDir{cameraDirData.FindMember("x")->value.GetFloat(),
               cameraDirData.FindMember("y")->value.GetFloat(),
               cameraDirData.FindMember("z")->value.GetFloat()};
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
    if (params.find("dir.z") != params.end()) {
      camDir.z = stoi(params["dir.z"]);
    }
    if (params.find("dir.x") != params.end()) {
      camDir.x = stoi(params["dir.x"]);
    }
    if (params.find("dir.y") != params.end()) {
      camDir.y = stoi(params["dir.y"]);
    }
    if (params.find("dir.z") != params.end()) {
      camDir.z = stoi(params["dir.z"]);
    }
  }
  ospray::cpp::Camera camera(cameraType);
  camera.set("aspect", (float)imgSize.x / (float)imgSize.y);
  camera.set("pos", camPos);
  camera.set("dir", camDir);
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
       << chrono::duration_cast<chrono::milliseconds>(
              chrono::steady_clock::now() - start)
              .count()
       << " ms " << endl;

  unsigned char *fb = (unsigned char *)framebuffer.map(OSP_FB_COLOR);
  vector<unsigned char> buf(fb, fb + imgSize.x * imgSize.y * 4);
  framebuffer.unmap(fb);

  return buf;
}