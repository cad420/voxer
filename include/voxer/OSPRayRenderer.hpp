#pragma once
#include <memory>
#include <voxer/Renderer.hpp>

namespace voxer {

class OSPRayRenderer : public Renderer {
public:
  OSPRayRenderer();
  Image render(const Scene &scene) override;

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};

} // namespace voxer
