#pragma once
#include "Rendering/IRenderer.hpp"
#include "Rendering/OpenGL/ShaderProgram.hpp"
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
  void render() final;
  auto get_colors() -> const Image & final;
  void clear_scene() override;

private:
  Image m_image;
  Camera m_camera;
  std::array<float, 3> m_background {};
  std::vector<std::shared_ptr<Volume>> m_volumes;
  std::vector<std::shared_ptr<Isosurface>> m_isosurfaces;
  std::unordered_map<StructuredGrid *, GLuint> m_dataset_cache;

  std::unique_ptr<ShaderProgram> m_position_program;
  std::unique_ptr<ShaderProgram> m_raycast_program;
  std::unique_ptr<ShaderProgram> m_screen_program;
  std::unique_ptr<ShaderProgram> m_essposition_program;
  std::unique_ptr<ShaderProgram> m_essraycast_program;

  EGLDisplay m_egl_display{};

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
VoxerIRenderer *voxer_get_backend() {
  return new voxer::OpenGLVolumeRenderer();
}
}