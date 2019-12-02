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
  Renderer() = default;
  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  virtual Image render(const Scene &scene) = 0;
  virtual ~Renderer() = default;
};

} // namespace voxer
