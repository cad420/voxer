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

  void set_camera(const Camera &) override;
  void add_volume(const std::shared_ptr<Volume> &) override;
  void add_isosurface(const std::shared_ptr<voxer::Isosurface> &) override;
  void render() final;
  auto get_colors() -> const Image & final;
  void clear_scene() override;

private:
  GL::GLTexture create_volume(StructuredGrid *dataset);

  Image image;
  EGLDisplay eglDpy;
  uint32_t width;
  uint32_t height;
  std::shared_ptr<GLContext> gl;

  Camera m_camera;
  std::vector<std::shared_ptr<Volume>> m_volumes;
  std::vector<std::shared_ptr<Isosurface>> m_isosurfaces;

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