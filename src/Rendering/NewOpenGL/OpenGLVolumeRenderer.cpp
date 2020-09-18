#include "OpenGLVolumeRenderer.hpp"
#include "Rendering/NewOpenGL/ShaderProgram.hpp"
#include "Rendering/NewOpenGL/shaders.hpp"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;

namespace {

const EGLint egl_config_attribs[] = {EGL_SURFACE_TYPE,
                                     EGL_PBUFFER_BIT,
                                     EGL_BLUE_SIZE,
                                     8,
                                     EGL_GREEN_SIZE,
                                     8,
                                     EGL_RED_SIZE,
                                     8,
                                     EGL_DEPTH_SIZE,
                                     8,
                                     EGL_RENDERABLE_TYPE,
                                     EGL_OPENGL_BIT,
                                     EGL_NONE};

const int pbuffer_width = 800;
const int pbuffer_height = 800;

const EGLint pbuffer_attribs[] = {
    EGL_WIDTH, pbuffer_width, EGL_HEIGHT, pbuffer_height, EGL_NONE,
};

void EGLCheck(const char *fn) {
  EGLint error = eglGetError();

  if (error != EGL_SUCCESS) {
    throw runtime_error(fn + to_string(error));
  }
}

GLuint create_tfcn_texture(voxer::TransferFunction &tfcn) {
  constexpr int tfcn_dim = 256;
  GLuint tfcn_texture = 0;
  vector<float> data;
  data.resize(tfcn_dim * 4);
  for (size_t i = 0; i < tfcn.colors.size(); i++) {
    auto &color = tfcn.colors[i];
    auto &opacity = tfcn.opacities[i];
    data[i * 4 + 0] = color[0];
    data[i * 4 + 1] = color[1];
    data[i * 4 + 2] = color[2];
    data[i * 4 + 3] = opacity;
  }

  glGenTextures(1, &tfcn_texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_1D, tfcn_texture);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, tfcn_dim, 0, GL_RGBA, GL_FLOAT,
               data.data());

  return tfcn_texture;
}

GLuint create_dataset_texture(voxer::StructuredGrid &dataset) {
  auto &info = dataset.info;
  auto &dimension = info.dimensions;
  auto &buffer = dataset.buffer;

  GLuint volume_texture = 0;
  glGenTextures(1, &volume_texture);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_3D, volume_texture);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, dimension[0], dimension[1],
               dimension[2], 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

  return volume_texture;
}

unsigned int volume_vertex_indices[36] = {0, 1, 2, 0, 2, 3, 0, 4, 1, 4, 5, 1,
                                          1, 5, 6, 6, 2, 1, 6, 7, 2, 7, 3, 2,
                                          7, 4, 3, 3, 4, 0, 4, 7, 6, 4, 6, 5};
float screen_quad_vertices[24] = {
    -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

    -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

} // namespace

namespace voxer {

OpenGLVolumeRenderer::OpenGLVolumeRenderer() {
  static const int MAX_DEVICES = 4;
  EGLDeviceEXT egl_devices[MAX_DEVICES];
  EGLint num_devices;

  auto eglQueryDevicesEXT =
      (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");
  eglQueryDevicesEXT(4, egl_devices, &num_devices);

  auto eglGetPlatformDisplayEXT =
      (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress(
          "eglGetPlatformDisplayEXT");

  m_egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT,
                                           egl_devices[0], nullptr);
  EGLCheck("eglGetDisplay");

  EGLint major, minor;
  eglInitialize(m_egl_display, &major, &minor);
  EGLCheck("eglInitialize");

  EGLint num_configs;
  EGLConfig egl_config;

  eglChooseConfig(m_egl_display, egl_config_attribs, &egl_config, 1,
                  &num_configs);
  EGLCheck("eglChooseConfig");

  EGLSurface egl_surface =
      eglCreatePbufferSurface(m_egl_display, egl_config, pbuffer_attribs);
  EGLCheck("eglCreatePbufferSurface");

  eglBindAPI(EGL_OPENGL_API);
  EGLCheck("eglBindAPI");

  const EGLint context_attri[] = {EGL_CONTEXT_MAJOR_VERSION,
                                  4,
                                  EGL_CONTEXT_MINOR_VERSION,
                                  6,
                                  EGL_CONTEXT_OPENGL_PROFILE_MASK,
                                  EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
                                  EGL_NONE};
  EGLContext egl_context = eglCreateContext(m_egl_display, egl_config,
                                            EGL_NO_CONTEXT, context_attri);
  EGLCheck("eglCreateContext");

  eglMakeCurrent(m_egl_display, egl_surface, egl_surface, egl_context);
  EGLCheck("eglMakeCurrent");

  if (!gladLoadGLLoader((void *(*)(const char *))(&eglGetProcAddress))) {
    throw runtime_error("Failed to load gl");
  }

  cout << "Detected " << num_devices << " devices, using first one: "
       << "OpenGL Version: " << GLVersion.major << "." << GLVersion.minor
       << endl;
}

OpenGLVolumeRenderer::~OpenGLVolumeRenderer() noexcept {
  eglTerminate(m_egl_display);
}

void OpenGLVolumeRenderer::set_camera(const Camera &camera) {
  m_camera = camera;
}

void OpenGLVolumeRenderer::add_volume(const std::shared_ptr<Volume> &volume) {
  // calculate volume data gradient

  // process tfcn

  // cache volume and gradient data
}

void OpenGLVolumeRenderer::add_isosurface(
    const std::shared_ptr<voxer::Isosurface> &isosurface) {}

void OpenGLVolumeRenderer::render() {
  auto distance =
      glm::length(glm::vec3(m_camera.pos[0], m_camera.pos[1], m_camera.pos[2]));
  auto projection =
      glm::ortho(-distance, distance, -distance, distance, 0.1f, 50.0f);
  auto view = glm::lookAt(
      glm::vec3(m_camera.pos[0], m_camera.pos[1], m_camera.pos[2]),
      glm::vec3(m_camera.target[0], m_camera.target[1], m_camera.target[2]),
      glm::vec3(m_camera.up[0], m_camera.up[1], m_camera.up[2]));
  auto model = glm::identity<glm::mat4>();
  // TODO
  model = glm::translate(model, glm::vec3(-0.5f, -0.5f, -0.5f));
  auto mvp_matrix = projection * view * model;

  glEnable(GL_DEPTH_TEST);
  // render pass 1
  glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
  glBindVertexArray(m_VAO);
  m_position_program->use();
  m_position_program->setMat4("MVPMatrix", mvp_matrix);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

  // render pass 2
  glDrawBuffer(GL_COLOR_ATTACHMENT1);
  glClear(GL_COLOR_BUFFER_BIT);
  glDepthFunc(GL_GREATER);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
  glDepthFunc(GL_LESS);

  // render pass 3
  glBindVertexArray(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  m_raycast_program->use();
  m_raycast_program->setMat4("projection", projection);
  m_raycast_program->setMat4("view", view);
  m_raycast_program->setMat4("model", model);
  m_raycast_program->setMat4("InverseView", glm::inverse(view));
  m_raycast_program->setMat4("InverseModel", glm::inverse(model));
  m_raycast_program->setVec3("cameraFront",
                             m_camera.target[0] - m_camera.pos[0],
                             m_camera.target[1] - m_camera.pos[1],
                             m_camera.target[2] - m_camera.pos[2]);

  if (m_camera.pos[0] <= -0.1 || m_camera.pos[0] >= 1.1 ||
      m_camera.pos[1] <= -0.1 || m_camera.pos[1] >= 1.1 ||
      m_camera.pos[2] <= -0.1 || m_camera.pos[2] >= 1.1) {
    m_raycast_program->setBool("inner", false);
  } else {
    m_raycast_program->setBool("inner", true);
    m_raycast_program->setVec3("cameraPos", m_camera.pos);
  }

  glDisable(GL_CULL_FACE);
  glBindVertexArray(m_VAO);
  // glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

  glFlush();
}

auto OpenGLVolumeRenderer::get_colors() -> const Image & { return m_image; }

void OpenGLVolumeRenderer::clear_scene() {}

void OpenGLVolumeRenderer::setup_resources() {
  m_position_program =
      make_unique<ShaderProgram>(shader::position_v, shader::position_f);
  m_raycast_program =
      make_unique<ShaderProgram>(shader::raycast_v, shader::raycast_f);
  m_screen_program =
      make_unique<ShaderProgram>(shader::screen_v, shader::screen_f);
  m_essposition_program =
      make_unique<ShaderProgram>(shader::essposition_v, shader::essposition_f);
  m_essraycast_program =
      make_unique<ShaderProgram>(shader::essraycast_v, shader::essraycast_f);
}

void OpenGLVolumeRenderer::setup_proxy_cude() {
  vector<float> volume_vertices(24, 0.0f);
  volume_vertices[1 * 3 + 0] = volume_vertices[2 * 3 + 0] =
      volume_vertices[5 * 3 + 0] = volume_vertices[6 * 3 + 0] = 1.0f;
  volume_vertices[4 * 3 + 1] = volume_vertices[5 * 3 + 1] =
      volume_vertices[6 * 3 + 1] = volume_vertices[7 * 3 + 1] = 1.0f;
  volume_vertices[2 * 3 + 2] = volume_vertices[3 * 3 + 2] =
      volume_vertices[6 * 3 + 2] = volume_vertices[7 * 3 + 2] = 1.0f;

  glGenVertexArrays(1, &m_VAO);
  glGenBuffers(1, &m_VBO);
  glGenBuffers(1, &m_EBO);

  glBindVertexArray(m_VAO);

  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, volume_vertices.data(),
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(volume_vertex_indices),
               volume_vertex_indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

void OpenGLVolumeRenderer::setup_framebuffer() {
  glGenFramebuffers(1,&m_FBO);
  glBindFramebuffer(GL_FRAMEBUFFER,m_FBO);

  glGenTextures(1,&entryTexture);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D,entryTexture);
  glTextureStorage2D(entryTexture,1,GL_RGBA32F,volumeInfo.ScreenW,volumeInfo.ScreenH);
  glBindImageTexture(1,entryTexture,0,GL_FALSE,0,GL_READ_ONLY,GL_RGBA32F);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,entryTexture,0);

  glGenRenderbuffers(1, &RBO);
  glBindRenderbuffer(GL_RENDERBUFFER, RBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, volumeInfo.ScreenW, volumeInfo.ScreenH); // use a single renderbuffer object for both a depth AND stencil buffer.
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);


  glGenTextures(1,&exitTexture);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D,exitTexture);
  glTextureStorage2D(exitTexture,1,GL_RGBA32F,volumeInfo.ScreenW,volumeInfo.ScreenH);
  glBindImageTexture(2,exitTexture,0,GL_FALSE,0,GL_READ_ONLY,GL_RGBA32F);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,exitTexture,0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){
    cout<<"Framebuffer object is not complete!"<<endl;
    exit(-1);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace voxer