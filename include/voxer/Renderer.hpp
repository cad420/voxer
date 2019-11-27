#pragma once
#include <map>
#include <string>
#include <vector>
#include <voxer/Camera.hpp>
#include <voxer/Image.hpp>
#include <voxer/Scene.hpp>

namespace voxer {

class Renderer {
public:
  Renderer() {}
  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  virtual Image render(const Scene &scene, const Camera &camera) = 0;
  virtual ~Renderer() {}
};

} // namespace voxer
