#include "Renderer.h"
#include "ospray/ospray_cpp.h"
#include <cstdlib>

using namespace std;
using namespace ospcommon;

extern DatasetManager datasets;

vector<unsigned char> Renderer::render(rapidjson::Value &values,
                                       map<string, string> *extraParams) {
  auto start = chrono::steady_clock::now();
  auto params = values.GetObject();

  auto &displayParams = params;
  auto &rendererParams = params["image"];

  ospray::cpp::Renderer renderer("scivis");
  ospray::cpp::Light light("scivis", "ambient");
  light.commit();
  auto lightHandle = light.handle();
  ospray::cpp::Data lights(1, OSP_LIGHT, &lightHandle);
  lights.commit();

  renderer.set("aoSamples", rendererParams["aoSamples"].GetInt());
  renderer.set("bgColor", 1.0f);
  renderer.set("lights", lights);

  // create OSP Camera
  auto cameraPosData = (rendererParams.FindMember("pos")->value).GetArray();
  auto cameraUpData = (rendererParams.FindMember("up")->value).GetArray();
  auto cameraDirData = (rendererParams.FindMember("dir")->value).GetArray();
  vec2i imgSize{displayParams["width"].GetInt(),
                displayParams["height"].GetInt()};

  vec3f camPos{cameraPosData[0].GetFloat(), cameraPosData[1].GetFloat(),
               cameraPosData[2].GetFloat()};
  vec3f camUp{cameraUpData[0].GetFloat(), cameraUpData[1].GetFloat(),
              cameraUpData[2].GetFloat()};
  vec3f camDir{cameraDirData[0].GetFloat(), cameraDirData[1].GetFloat(),
               cameraDirData[2].GetFloat()};
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

  ospray::cpp::Camera camera("perspective");
  camera.set("aspect", (float)imgSize.x / (float)imgSize.y);
  camera.set("pos", camPos);
  camera.set("dir", camDir);
  camera.set("up", camUp);
  camera.commit();
  renderer.set("camera", camera);

  auto modelsParams = rendererParams["model"].GetArray();
  ospray::cpp::Model world;
  for (auto &modelParams : modelsParams) {
    auto &volumeParams = modelParams["volume"];
    auto &tfcnParams = volumeParams["transferfunction"];
    auto &datasetParams = volumeParams["dataset"];
    auto volumeName = datasetParams["source"].GetString();
    auto volume = datasets.get(volumeName);

    vec2f valueRange{0, 255};
    if (datasetParams["source"] == "magnetic") {
      valueRange.x = 0.44;
      valueRange.y = 0.77;
    }
    vector<vec3f> colors;
    vector<float> opacities;
    const auto &points = tfcnParams["tfcn"].GetArray();
    for (auto &point : points) {
      const auto hex = string(point["color"].GetString());
      const auto opacity = point["y"].GetFloat();
      colors.push_back(
          vec3f{strtol(hex.substr(1, 2).c_str(), nullptr, 16) * 1.0f / 255,
                strtol(hex.substr(3, 2).c_str(), nullptr, 16) * 1.0f / 255,
                strtol(hex.substr(5, 2).c_str(), nullptr, 16) * 1.0f / 255});
      opacities.push_back(opacity);
    }
    ospray::cpp::Data colorsData(colors.size(), OSP_FLOAT3, colors.data());
    ospray::cpp::Data opacityData(opacities.size(), OSP_FLOAT,
                                  opacities.data());
    colorsData.commit();
    opacityData.commit();
    volume.tfcn.set("valueRange", valueRange);
    volume.tfcn.set("colors", colorsData);
    volume.tfcn.set("opacities", opacityData);
    volume.tfcn.commit();
    volume.volume.set("voxelRange", valueRange);
    volume.volume.commit();

    world.addVolume(volume.volume);
    vector<box3f> regions{volume.bounds};
    ospray::cpp::Data regionData(regions.size() * 2, OSP_FLOAT3,
                                 regions.data());
    world.set("regions", regionData);

    auto geometriesParams = modelParams["geometry"].GetArray();
    for (auto &geometryParams : geometriesParams) {
      auto geoType = string(geometryParams["type"].GetString());
      ospray::cpp::Geometry geometry(geoType.c_str());
      if (geoType == "triangles") {
      } else if (geoType == "spheres") {
      } else if (geoType == "cylinders") {
      } else if (geoType == "isosurfaces") {
        auto isovalue = geometryParams["isovalues"].GetFloat();
        ospray::cpp::Data isovalues(1, OSP_FLOAT, &isovalue);
        isovalues.commit();
        geometry.set("isovalues", isovalues);
        geometry.set("volume", volume.volume);
      } else if (geoType == "streamlines") {
      } else if (geoType == "slices") {
      }
      geometry.commit();
      world.addGeometry(geometry);
    }
  }
  world.commit();
  renderer.set("model", world);
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