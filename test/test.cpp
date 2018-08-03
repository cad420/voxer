#include "../ParallelRenderer/ConfigManager.h"
#include "../ParallelRenderer/DatasetManager.h"
#include "../ParallelRenderer/Encoder.h"
#include "../ParallelRenderer/Renderer.h"
#include "third_party/rapidjson/document.h"
#include <fstream>

using namespace std;

DatasetManager datasets;
ConfigManager configs;
Renderer renderer;
Encoder encoder;

int main(int argc, const char **argv) {
  string datasetFile = "/home/ukabuer/workspace/vovis/configs/datasets.json";
  string configureFile = "/home/ukabuer/workspace/vovis/test/configs/1.json";
  OSPError init_error = ospInit(&argc, argv);
  try {
    datasets.load(datasetFile);
    configs.load(configureFile);
    auto &config = configs.get("fake-id-1");
    cout << "get config" << endl;
    auto data = renderer.render(config);
    cout << "rendered, " << data.size() << endl;
    auto img = encoder.encode(data, config.size, "JPEG");
    ofstream imageFile;
    imageFile.open("result.jpg", ios::out | ios::binary);
    imageFile.write((char *)img.data(), img.size());
    imageFile.close();
  } catch (string err) {
    cout << "error: " << err << endl;
  }
  return 1;
}