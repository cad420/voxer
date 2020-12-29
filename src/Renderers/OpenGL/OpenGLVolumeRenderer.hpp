#pragma once
#define MESA_EGL_NO_X11_HEADERS
#define EGL_NO_X11
#define EGL_EGLEXT_PROTOTYPES
#include "Renderers/IRenderer.hpp"
#include "Renderers/OpenGL/OpenGLVolumeCache.hpp"
#include "Renderers/OpenGL/ShaderProgram.hpp"
#include <EGL/egl.h>
#include <unordered_map>
#include <voxer/Data/Image.hpp>

namespace voxer {

class OpenGLVolumeRenderer : public VoxerIRenderer {
public:
  OpenGLVolumeRenderer();
  ~OpenGLVolumeRenderer() noexcept override;

  void set_camera(const Camera &) override;
  void set_background(float r, float g, float b) noexcept override;
  void add_volume(const std::shared_ptr<Volume> &) override;
  void add_isosurface(const std::shared_ptr<voxer::Isosurface> &) override;
  void render() override;
  auto get_colors() -> const Image & override;
  void clear_scene() override;
  bool has_cache(StructuredGrid *data) const noexcept override;

private:
  OpenGLVolumeCache *m_cache;
  Image m_image;
  Camera m_camera;
  std::array<float, 3> m_background{};
  std::vector<std::shared_ptr<Volume>> m_volumes;
  std::vector<std::shared_ptr<Isosurface>> m_isosurfaces;

  std::unique_ptr<ShaderProgram> m_position_program;
  std::unique_ptr<ShaderProgram> m_raycast_program;
  std::unique_ptr<ShaderProgram> m_screen_program;
  std::unique_ptr<ShaderProgram> m_essposition_program;
  std::unique_ptr<ShaderProgram> m_essraycast_program;

  EGLContext m_egl_context{};
  EGLSurface m_egl_surface{};

  GLuint m_VBO = 0;
  GLuint m_EBO = 0;
  GLuint m_VAO = 0;

  GLuint m_entry_texture = 0;
  GLuint m_exit_texture = 0;
  GLuint m_RBO = 0;
  GLuint m_FBO = 0;

  void setup_context();

  void setup_resources();

  void setup_proxy_cude();
};

} // namespace voxer

extern "C" {
VoxerIRenderer *voxer_get_renderer();
}