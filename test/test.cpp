#include "ParallelRenderer/ConfigManager.h"
#include "ParallelRenderer/DatasetManager.h"
#include "ParallelRenderer/Encoder.h"
#include "ParallelRenderer/Renderer.h"
#include "ParallelRenderer/util/Debugger.h"
#include "third_party/rapidjson/document.h"
#include <fstream>

using namespace std;

DatasetManager datasets;
ConfigManager configs;
Renderer renderer;
Encoder encoder;

int main(int argc, const char **argv) {
  Debugger debug("main");
  string datasetFile = "/home/ukabuer/workspace/vovis/configs/datasets.json";
  string configureFile = "/home/ukabuer/workspace/vovis/test/configs/1.json";
  OSPError init_error = ospInit(&argc, argv);
  try {
    datasets.load(datasetFile);
    configs.load(configureFile);
    auto &config = configs.get("fake-id-1");
    debug.log("get config");
    auto data = renderer.render(config);
    debug.log("rendered");
    auto img = encoder.encode(data, config.size, "JPEG");
    debug.log("encoded");
    ofstream imageFile;
    imageFile.open("result.jpg", ios::out | ios::binary);
    imageFile.write((char *)img.data(), img.size());
    imageFile.close();
    debug.log("saved");
  } catch (string err) {
    debug.log("error: " + err);
  }
  return 1;
}