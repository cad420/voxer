#include "VoxerWorkerApplication.hpp"

using namespace voxer::remote;

int main(int argc, char **argv) {
  VoxerWorkerApplication app;
  return app.run(argc, argv);
}
