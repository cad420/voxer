#pragma once
#include <memory>
#include <voxer/Renderer.hpp>

namespace voxer {

class OSPRayRenderer : public Renderer {
public:
  explicit OSPRayRenderer(const DatasetStore &datasets);
  ~OSPRayRenderer() override;

  Image render(const Scene &scene) override;

private:
  struct Impl;
  std::unique_ptr<Impl> impl;
};

} // namespace voxer
