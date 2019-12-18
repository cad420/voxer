#pragma once
#include <memory>
#include <voxer/Renderer.hpp>

namespace voxer {

class OSPRayRenderer : public Renderer {
public:
  explicit OSPRayRenderer(DatasetStore &datasets);
  ~OSPRayRenderer() override;

  Image render(const Scene &scene) override;

private:
  struct Cache;
  std::unique_ptr<Cache> cache;
};

} // namespace voxer
