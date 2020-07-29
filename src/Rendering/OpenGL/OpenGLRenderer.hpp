#pragma once
#define None None
#include "GLContext.hpp"
#include "Rendering/IRenderer.hpp"
#include <EGL/egl.h>
#include <unordered_map>
#include <voxer/DatasetStore.hpp>
#include <voxer/Image.hpp>
#include <voxer/Scene.hpp>

namespace voxer {

class OpenGLRenderer final : public VoxerIRenderer {
public:
  OpenGLRenderer();
  ~OpenGLRenderer() final;

  void render(const Scene &scene, DatasetStore &datasets) final;
  auto get_colors() -> const Image & final;

private:
  GL::GLTexture create_volume(const Dataset &dataset);

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

extern "C" {
VoxerIRenderer *voxer_get_backend() { return new voxer::OpenGLRenderer(); }
}