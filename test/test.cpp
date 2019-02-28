#include "ParallelRenderer/ConfigManager.h"
#include "ParallelRenderer/DatasetManager.h"
#include "ParallelRenderer/Encoder.h"
#include "ParallelRenderer/Renderer.h"
#include "ParallelRenderer/UserManager.h"
#include "ParallelRenderer/util/Debugger.h"
#include "third_party/rapidjson/document.h"
#include <fstream>

using namespace std;

DatasetManager datasets;
UserManager users;
ConfigManager configs;
Encoder encoder;
Debugger debug("main");

void render() {
  auto &config = configs.get("fake-id-1");
  debug.log("get config");
  unique_ptr<Renderer> renderer;
  bool isRGBA = true;
  if (config.volumesToRender.size() > 1) {
    renderer.reset(new VTKRenderer());
    isRGBA = false;
  } else {
    renderer.reset(new OSPRayRenderer());
  }
  auto data = renderer->render(config);
  debug.log("rendered");
  auto img = encoder.encode(data, config.size, "JPEG", isRGBA);
  debug.log("encoded");
  ofstream imageFile;
  imageFile.open("result.jpg", ios::out | ios::binary);
  imageFile.write((char *)img.data(), img.size());
  imageFile.close();
  debug.log("saved");
}

int main(int argc, const char **argv) {
  string datasetFile = "/home/ukabuer/workspace/vovis/configs/datasets.json";
  OSPError init_error = ospInit(&argc, argv);
  if (argc < 2) {
    cout << "Usage: Test /path/to/config.json" << endl;
    return 1;
  }

  string configureFile = argv[1];
  try {
    datasets.load(datasetFile);
    configs.load(configureFile);
    auto& user = users.get("tester");
    user.load("heptane");
    user.load("lsabel-TCf-05");
    user.load("lsabel-Pf-05");
    debug.log("loaded");
    for (int i = 0; i < 2; i++)
      render();
  } catch (string err) {
    cout << "error: " << err << endl;
  } catch (const char* err) {
    cout << "error: " << err << endl;
  }
  return 0;
}

