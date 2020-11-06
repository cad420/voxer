#include "Server.hpp"
#include "VoxerRemoteApplication.hpp"

using namespace std;
using namespace voxer::remote;

int main(int argc, char **argv) {
  VoxerRemoteApplication app;
  return app.run(argc, argv);
}
