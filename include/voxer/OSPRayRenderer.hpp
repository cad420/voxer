#pragma once
#include <memory>
#include <voxer/Renderer.hpp>

namespace voxer {

class OSPRayRenderer : public Renderer {
public:
  OSPRayRenderer();
  Image render(const Scene &scene) override;
};

} // namespace voxer
