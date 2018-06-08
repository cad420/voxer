#include "Renderer.h"
#include "ospray/ospray_cpp.h"
#include <cstdlib>

using namespace std;
using namespace ospcommon;

extern DatasetManager datasets;

void setupVolume(gensv::LoadedVolume &volume, rapidjson::Value &volumeParams, int timestep = 0) {
  if (volumeParams["type"] == "structured") {
    auto &tfcnParams = volumeParams["transferfunction"];
    auto &datasetParams = volumeParams["dataset"];
    auto volumeName = string(datasetParams["source"].GetString());
    if (timestep != 0) {
      volumeName += to_string(timestep);
    } else if (datasetParams.HasMember("timestep")) {
      volumeName += to_string(datasetParams["timestep"].GetInt());
    }
    auto &dataset = datasets.get(volumeName.c_str());
    const auto upper = ospcommon::vec3f(dataset.dimensions);
    const auto halfLength = dataset.dimensions / 2;
    gensv::loadVolume(volume, dataset.buffer, dataset.dimensions, dataset.dtype,
                      dataset.sizeForDType);
    volume.bounds.lower -= ospcommon::vec3f(halfLength);
    volume.bounds.upper -= ospcommon::vec3f(halfLength);
    volume.volume.set("gridOrigin",
                      volume.ghostGridOrigin - ospcommon::vec3f(halfLength));
    vec2f valueRange{0, 255};
    if (datasetParams["source"] == "magnetic") {
      valueRange.x = 0.44;
      valueRange.y = 0.77;
    }
    vector<vec3f> colors;
    vector<float> opacities;
    const auto &points = tfcnParams["tfcn"].GetArray();
    for (auto point = points.Begin(); point != points.End() - 1; ++point) {
      auto &startParams = *point;
      auto &endParams = *(point + 1);
      auto start = startParams["x"].GetFloat();
      auto end = endParams["x"].GetFloat();
      auto startHex = string(startParams["color"].GetString());
      auto startR =
          strtol(startHex.substr(1, 2).c_str(), nullptr, 16) * 1.0f / 255;
      auto startG =
          strtol(startHex.substr(3, 2).c_str(), nullptr, 16) * 1.0f / 255;
      auto startB =
          strtol(startHex.substr(5, 2).c_str(), nullptr, 16) * 1.0f / 255;
      auto endHex = string(endParams["color"].GetString());
      auto endR = strtol(endHex.substr(1, 2).c_str(), nullptr, 16) * 1.0f / 255;
      auto endG = strtol(endHex.substr(3, 2).c_str(), nullptr, 16) * 1.0f / 255;
      auto endB = strtol(endHex.substr(5, 2).c_str(), nullptr, 16) * 1.0f / 255;
      auto startOpa = startParams["y"].GetFloat();
      auto endOpa = endParams["y"].GetFloat();
      auto period = 100 * (end - start);
      auto step = 1.0f / period;
      auto rDiff = (endR - startR) * step;
      auto gDiff = (endG - startG) * step;
      auto bDiff = (endB - startB) * step;
      auto opaDiff = (endOpa - startOpa) * step;
      for (auto j = 0; j < period; j++) {
        opacities.push_back(startOpa + j * opaDiff);
        colors.push_back(
            vec3f{startR + j * rDiff, startG + j * gDiff, startB + j * bDiff});
      }
    }
    ospray::cpp::Data colorsData(colors.size(), OSP_FLOAT3, colors.data());
    ospray::cpp::Data opacityData(opacities.size(), OSP_FLOAT,
                                  opacities.data());
    colorsData.commit();
    opacityData.commit();
    volume.tfcn.set("colors", colorsData);
    volume.tfcn.set("opacities", opacityData);
    volume.tfcn.set("valueRange", valueRange);
    volume.tfcn.commit();
  } else if (volumeParams["type"] == "clipping") {
    auto &upperParams = volumeParams["upper"];
    auto &lowerParams = volumeParams["lower"];
    auto upper = vec3f(upperParams[0].GetFloat(), upperParams[1].GetFloat(),
                       upperParams[2].GetFloat());
    auto lower = vec3f(lowerParams[0].GetFloat(), lowerParams[1].GetFloat(),
                       lowerParams[2].GetFloat());
    auto &_volumeParams = volumeParams["volume1"];
    setupVolume(volume, _volumeParams);
    volume.volume.set("volumeClippingBoxLower", upper);
    volume.volume.set("volumeClippingBoxUpper", lower);
  } else if (volumeParams["type"] == "diff") {
    auto &volume1Params = volumeParams["volume1"];
    auto &volume2Params = volumeParams["volume2"];
    gensv::LoadedVolume volume1;
    gensv::LoadedVolume volume2;
    setupVolume(volume1, volume1Params);
    setupVolume(volume2, volume2Params);
    auto buffer1 = *volume1.buffer;
    auto buffer2 = *volume2.buffer;
    auto dimensions = buffer1.size() > buffer2.size() ? *volume1.dimensions
                                                      : *volume2.dimensions;
    auto limit =
        buffer1.size() > buffer2.size() ? buffer2.size() : buffer1.size();
    const auto upper = ospcommon::vec3f(dimensions);
    const auto halfLength = ospcommon::vec3i(dimensions) / 2;
    auto ptr = new vector<unsigned char>(dimensions.x * dimensions.y * dimensions.z);
    cout << 0 << endl;
    for (auto i = 0; i < limit; i++) {
      (*ptr)[i] = (buffer1[i] - buffer2[i] + 255) / 2;
    }
    gensv::loadVolume(volume, *ptr, dimensions, "uchar", 1);
    volume.isNewBuffer = true;
    volume.bounds.lower -= ospcommon::vec3f(halfLength);
    volume.bounds.upper -= ospcommon::vec3f(halfLength);
    volume.volume.set("transferFunction", volume1.tfcn);
    volume.volume.set("gridOrigin",
                      volume.ghostGridOrigin - ospcommon::vec3f(halfLength));
  } else if (volumeParams["type"] == "transform") {
    auto x = volumeParams["x"].GetFloat();
    auto y = volumeParams["y"].GetFloat();
    auto z = volumeParams["z"].GetFloat();
    auto &_volumeParams = volumeParams["volume1"];
    auto &datasetParams = _volumeParams["dataset"];
    setupVolume(volume, _volumeParams);
    const auto halfLength = *volume.dimensions / 2;
    auto origin = volume.ghostGridOrigin - ospcommon::vec3f(halfLength) +
                  ospcommon::vec3f(x, y, z);
    volume.volume.set("gridOrigin", origin);
  }
}

vector<unsigned char> Renderer::render(rapidjson::Value &values,
                                       map<string, string> *extraParams) {
  auto start = chrono::steady_clock::now();
  auto params = values.GetObject();

  auto &displayParams = params;
  auto &rendererParams = params["image"];
  ospray::cpp::Renderer renderer("scivis");

  vector<OSPLight> lightList;
  if (rendererParams.HasMember("light")) {
    auto lightsParams = rendererParams["light"].GetArray();
    for (auto &lightParams : lightsParams) {
      auto type = lightParams["type"].GetString();
      ospray::cpp::Light light("scivis", type);
      if (type == "point") {

      } else if (type == "spot") {

      } else if (type == "ambient") {

      }
      lightList.push_back(light.handle());
    }
  } else {
    ospray::cpp::Light light("scivis", "ambient");
    lightList.push_back(light.handle());
  }

  ospray::cpp::Data lights(lightList.size(), OSP_LIGHT, lightList.data());
  lights.commit();
  renderer.set("lights", lights);

  renderer.set("aoSamples", 0);
  renderer.set("bgColor", 1.0f);

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
  int timestep = 0;
  if (extraParams) {
    auto params = *extraParams;
    if (params.find("width") != params.end()) {
      imgSize.x = stoi(params["width"]);
    }
    if (params.find("height") != params.end()) {
      imgSize.y = stoi(params["height"]);
    }
    if (params.find("pos.x") != params.end()) {
      camPos.x = stof(params["pos.x"]);
    }
    if (params.find("pos.y") != params.end()) {
      camPos.y = stof(params["pos.y"]);
    }
    if (params.find("pos.z") != params.end()) {
      camPos.z = stof(params["pos.z"]);
    }
    if (params.find("dir.x") != params.end()) {
      camDir.x = stof(params["dir.x"]);
    }
    if (params.find("dir.y") != params.end()) {
      camDir.y = stof(params["dir.y"]);
    }
    if (params.find("dir.z") != params.end()) {
      camDir.z = stof(params["dir.z"]);
    }
    if (params.find("up.x") != params.end()) {
      camUp.x = stof(params["up.x"]);
    }
    if (params.find("up.y") != params.end()) {
      camUp.y = stof(params["up.y"]);
    }
    if (params.find("up.z") != params.end()) {
      camUp.z = stof(params["up.z"]);
    }
    if (params.find("timestep") != params.end()) {
      timestep = stoi(params["timestep"]);
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
  vector<vector<unsigned char> *> bufferToBeFree;
  for (auto &modelParams : modelsParams) {
    auto &volumeParams = modelParams["volume"];
    gensv::LoadedVolume volume;
    setupVolume(volume, volumeParams, timestep);
    if (volume.isNewBuffer) {
      bufferToBeFree.push_back(volume.buffer);
    }
    volume.volume.commit();
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
        if (geoType == "triangle") {
        } else if (geoType == "sphere") {
        } else if (geoType == "isosurface") {
          auto isovalue = geometryParams["isovalues"].GetFloat();
          ospray::cpp::Data isovalues(1, OSP_FLOAT, &isovalue);
          isovalues.commit();
          geometry.set("isovalues", isovalues);
          geometry.set("volume", volume.volume);
        } else if (geoType == "slice") {
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

  for(auto buffer : bufferToBeFree) {
    delete buffer;
  }
  return buf;
}