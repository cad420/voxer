#pragma once
#include <map>
#include <string>
#include <vector>
#include <voxer/DatasetStore.hpp>
#include <voxer/Image.hpp>
#include <voxer/Scene.hpp>

namespace voxer {

class Renderer {
public:
  explicit Renderer(DatasetStore &datasets) : datasets(datasets){};
  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;
  Renderer(const Renderer &&) = delete;
  Renderer &operator=(Renderer &&) = delete;

  virtual Image render(const Scene &scene) = 0;
  virtual ~Renderer() = default;

protected:
  DatasetStore &datasets;
};

} // namespace voxer
