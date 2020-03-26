#pragma once
#define None None
#include "Rendering/IRenderingContext.hpp"
#include "Rendering/OpenGL/GLContext.hpp"
#include <EGL/egl.h>
#include <unordered_map>

namespace voxer {

class RenderingContextOpenGL : public IRenderingContext {
public:
  RenderingContextOpenGL();
  ~RenderingContextOpenGL() final;

  void render(const Scene &scene, DatasetStore &datasets) final;
  auto get_colors() -> const Image & final;

private:
  Image image;
  EGLDisplay eglDpy;
  uint32_t width;
  uint32_t height;
  std::shared_ptr<GLContext> gl;

  GL::GLProgram positionGenerateProgram;
  GL::GLProgram raycastingProgram;
  GL::GLProgram screenQuadProgram;
  GL::GLVertexArray vao;
  GL::GLBuffer vbo;
  GL::GLBuffer ebo;

  std::unordered_map<SceneDataset, GL::GLTexture, SceneDatasetHasher>
      volume_cache;
};

} // namespace voxer
