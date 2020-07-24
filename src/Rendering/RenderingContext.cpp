#include "Rendering/IRenderingContext.hpp"
#include "utils/Logger.hpp"
#include <chrono>
#include <dlfcn.h>
#include <functional>
#include <memory>
#include <voxer/RenderingContext.hpp>

using namespace std;

namespace voxer {

static Logger logger("renderer");

using GetRenderingBackend = VoxerIRenderingContext *(*)();

RenderingContext::~RenderingContext() = default;

RenderingContext::RenderingContext(RenderingContext::Type type) {
  void *lib = nullptr;
  switch (type) {
  case Type::OSPRay: {
    lib = dlopen("libvoxer_backend_ospray.so", RTLD_NOW | RTLD_GLOBAL);
    break;
  }
  case Type::OpenGL: {
    lib = dlopen("libvoxer_backend_gl.so", RTLD_NOW | RTLD_GLOBAL);
  }
  }
  if (lib == nullptr) {
    throw runtime_error(dlerror());
  }

  void *symbol = dlsym(lib, "voxer_get_backend");
  if (symbol == nullptr) {
    throw runtime_error("Cannot find symbol `voxer_get_backend`");
  }

  std::function<VoxerIRenderingContext *()> get_backend =
      reinterpret_cast<GetRenderingBackend>(symbol);
  auto context = get_backend();
  impl.reset(context);
}

void RenderingContext::render(const Scene &scene, DatasetStore &datasets) {
  if (this->impl == nullptr) {
    return;
  }

  auto start = chrono::steady_clock::now();

  this->impl->render(scene, datasets);

  const auto delta = chrono::duration_cast<chrono::milliseconds>(
      chrono::steady_clock::now() - start);
  logger.info(to_string(delta.count()) + " ms");
}

auto RenderingContext::get_colors() -> const Image & {
  return this->impl->get_colors();
}

} // namespace voxer