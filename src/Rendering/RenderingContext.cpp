#include "Rendering/IRenderingContext.hpp"
#include "Rendering/OSPRay/RenderingContextOSPRay.hpp"
#include "Rendering/OpenGL/RenderingContextOpenGL.hpp"
#include "utils/Logger.hpp"
#include <chrono>
#include <memory>
#include <voxer/RenderingContext.hpp>

using namespace std;

namespace voxer {

static Logger logger("renderer");

RenderingContext::~RenderingContext() = default;

RenderingContext::RenderingContext(RenderingContext::Type type) {
  switch (type) {
  case Type::OSPRay: {
    impl = make_unique<RenderingContextOSPRay>();
    break;
  }
  case Type::OpenGL: {
    impl = make_unique<RenderingContextOpenGL>();
  }
  }
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