#include "Renderer.h"
#include "ospray/ospray_cpp.h"
#include <cstdlib>

using namespace std;
using namespace ospcommon;

extern DatasetManager datasets;

void setupVolume(gensv::LoadedVolume *volume,rapidjson::Value &volumeParams) {
  auto &tfcnParams = volumeParams["transferfunction"];
  auto &datasetParams = volumeParams["dataset"];
  auto volumeName = string(datasetParams["source"].GetString());
  if (datasetParams.HasMember("timestep")) {
    volumeName += to_string(datasetParams["timestep"].GetInt());
  }
  auto &dataset = datasets.get(volumeName.c_str());
  const auto upper = ospcommon::vec3f(dataset.dimensions);
  const auto halfLength = ospcommon::vec3i(dataset.dimensions) / 2;
  gensv::loadVolume(volume, dataset.buffer, ospcommon::vec3i(dataset.dimensions),
                        dataset.dtype, dataset.sizeForDType);
  volume->bounds.lower -= ospcommon::vec3f(halfLength);
  volume->bounds.upper -= ospcommon::vec3f(halfLength);
  volume->volume.set("gridOrigin",
                    volume->ghostGridOrigin - ospcommon::vec3f(halfLength));

  vec2f valueRange{0, 255};
  if (datasetParams["source"] == "magnetic") {
    valueRange.x = 0.44;
    valueRange.y = 0.77;
  }
  // vector<vec3f> colors;
  // vector<float> opacities;
  // const auto &points = tfcnParams["tfcn"].GetArray();
  // for (auto &point : points) {
  //   const auto hex = string(point["color"].GetString());
  //   const auto opacity = point["y"].GetFloat();
  //   colors.push_back(
  //       vec3f{strtol(hex.substr(1, 2).c_str(), nullptr, 16) * 1.0f / 255,
  //             strtol(hex.substr(3, 2).c_str(), nullptr, 16) * 1.0f / 255,
  //             strtol(hex.substr(5, 2).c_str(), nullptr, 16) * 1.0f / 255});
  //   opacities.push_back(opacity);
  // }
  const std::vector<vec3f> colors{
      vec3f(0, 0, 0.56), vec3f(0, 0, 1), vec3f(0, 1, 1),  vec3f(0.5, 1, 0.5),
      vec3f(1, 1, 0),    vec3f(1, 0, 0), vec3f(0.5, 0, 0)};
  const std::vector<float> opacities{0.0001f, 1.0f};
  ospray::cpp::Data colorsData(colors.size(), OSP_FLOAT3, colors.data());
  ospray::cpp::Data opacityData(opacities.size(), OSP_FLOAT, opacities.data());
  colorsData.commit();
  opacityData.commit();
  volume->tfcn.set("colors", colorsData);
  volume->tfcn.set("opacities", opacityData);
  volume->tfcn.set("valueRange", valueRange);
  volume->tfcn.commit();
}

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

  renderer.set("aoSamples", 0);
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
    gensv::LoadedVolume volume;
    if (volumeParams["type"] == "structured") {
      setupVolume(&volume, volumeParams);
      volume.volume.commit();
    } else if (volumeParams["type"] == "clipping") {
      auto &upperParams = volumeParams["upper"];
      auto &lowerParams = volumeParams["lower"];
      auto upper = vec3f(upperParams[0].GetFloat(), upperParams[1].GetFloat(),
                         upperParams[2].GetFloat());
      auto lower = vec3f(lowerParams[0].GetFloat(), lowerParams[1].GetFloat(),
                         lowerParams[2].GetFloat());
      auto &_volumeParams = volumeParams["volume1"];
      setupVolume(&volume, _volumeParams);
      volume.volume.set("volumeClippingBoxLower", upper);
      volume.volume.set("volumeClippingBoxUpper", lower);
      volume.volume.commit();
    } else if (volumeParams["type"] == "diff") {
      auto &volume1Params = volumeParams["volume1"];
      auto &volume2Params = volumeParams["volume2"];
      setupVolume(&volume, volumeParams);
      volume.volume.commit();
    }
    world.addVolume(volume.volume);
    vector<box3f> regions{volume.bounds};
    ospray::cpp::Data regionData(regions.size() * 2, OSP_FLOAT3,
                                 regions.data());
    world.set("regions", regionData);
    if (modelParams.HasMember("geometry")) {
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