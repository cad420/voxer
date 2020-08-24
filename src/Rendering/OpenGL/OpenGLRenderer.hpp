#pragma once
#define None None
#include "GLContext.hpp"
#include "Rendering/IRenderer.hpp"
#include <EGL/egl.h>
#include <unordered_map>
#include <voxer/Data/Image.hpp>

namespace voxer {

class OpenGLRenderer final : public VoxerIRenderer {
public:
  OpenGLRenderer();
  ~OpenGLRenderer() override;

  void render() override;
  auto get_colors() -> const Image & override;

private:
  GL::GLTexture create_volume(const StructuredGrid &dataset);

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

  std::unordered_map<StructuredGrid *, GL::GLTexture> volume_cache;
};

} // namespace voxer

extern "C" {
VoxerIRenderer *voxer_get_backend() { return new voxer::OpenGLRenderer(); }
}