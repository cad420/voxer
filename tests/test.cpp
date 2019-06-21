#include "third_party/rapidjson/document.h"
#include "voxer/encoders/Encoder.hpp"
#include "voxer/managers/ConfigManager.hpp"
#include "voxer/managers/DatasetManager.hpp"
#include "voxer/managers/UserManager.hpp"
#include "voxer/renderers/Renderer.hpp"
#include "voxer/utils/Debugger.hpp"
#define CATCH_CONFIG_RUNNER
#include "third_party/catch.hpp"
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

unsigned int Factorial( unsigned int number ) {
    return number <= 1 ? number : Factorial(number-1)*number;
}


TEST_CASE( "Factorials are computed", "[factorial]" ) {
    REQUIRE( Factorial(1) == 1 );
    REQUIRE( Factorial(2) == 2 );
    REQUIRE( Factorial(3) == 6 );
    REQUIRE( Factorial(10) == 3628800 );
}
// TEST_CASE("Test basic rendering", "[rendering]") {
//   render();
//   cout << 1 << endl;
//   REQUIRE(1 == 1);
// }

int main(int argc, const char **argv) {
  OSPError init_error = ospInit(&argc, argv);

  if (argc < 2) {
    cout << "Usage: " << argv[0]
         << " /path/to/dataset.json /path/to/config.json" << endl;
    return 1;
  }

  if (argc < 3) {
    cout << "Usage: " << argv[0]
         << " /path/to/dataset.json /path/to/config.json" << endl;
    return 1;
  }

  string datasetFile = argv[1];
  string configureFile = argv[2];
  Catch::Session session;
  auto ret = session.applyCommandLine( argc, argv );
    if (ret) {
        return ret;
    }
  try {
    datasets.load(datasetFile);
    configs.load(configureFile);
    auto &user = users.get("tester");
    user.load("heptane");
    debug.log("loaded");

    int result = session.run();
  } catch (string err) {
    cout << "error: " << err << endl;
  } catch (const char *err) {
    cout << "error: " << err << endl;
  }
  return 0;
}

