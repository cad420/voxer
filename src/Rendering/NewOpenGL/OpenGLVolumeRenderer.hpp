#pragma once
#include "Rendering/IRenderer.hpp"
#include "Rendering/NewOpenGL/ShaderProgram.hpp"
#include <EGL/egl.h>
#include <voxer/Data/Image.hpp>

namespace voxer {

struct VolumeRenderingInfo {
  GLuint m_tfcn_texture = 0;
  GLuint m_volume_texture = 0;
  GLuint m_volume_gradient_texture = 0;
};

class OpenGLVolumeRenderer : public VoxerIRenderer {
public:
  OpenGLVolumeRenderer();
  ~OpenGLVolumeRenderer() noexcept override;

  void set_camera(const Camera &) override;
  void add_volume(const std::shared_ptr<Volume> &) override;
  void add_isosurface(const std::shared_ptr<voxer::Isosurface> &) override;
  void render() final;
  auto get_colors() -> const Image & final;
  void clear_scene() override;

private:
  Camera m_camera;
  Image m_image;
  std::unique_ptr<ShaderProgram> m_position_program;
  std::unique_ptr<ShaderProgram> m_raycast_program;
  std::unique_ptr<ShaderProgram> m_screen_program;
  std::unique_ptr<ShaderProgram> m_essposition_program;
  std::unique_ptr<ShaderProgram> m_essraycast_program;

  EGLDisplay m_egl_display;
  GLuint m_VBO = 0;
  GLuint m_VAO = 0;
  GLuint m_EBO = 0;
  GLuint m_FBO = 0;

  void setup_resources();

  void setup_proxy_cude();

  void setup_framebuffer();
};

} // namespace voxer

extern "C" {
VoxerIRenderer *voxer_get_backend() {
  return new voxer::OpenGLVolumeRenderer();
}
}