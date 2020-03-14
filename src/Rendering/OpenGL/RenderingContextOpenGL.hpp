#pragma once
#include "Rendering/IRenderingContext.hpp"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

namespace voxer {

class RenderingContextOpenGL : public IRenderingContext {
public:
  RenderingContextOpenGL();
  ~RenderingContextOpenGL() final;

  void render(const Scene &scene, DatasetStore &datasets) final;
  auto get_colors() -> const Image & final;

private:
  Image image;
  GLFWwindow *window;
  uint32_t width;
  uint32_t height;
};

} // namespace voxer
