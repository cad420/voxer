#include "OpenGLVolumeRenderer.hpp"
#include "Rendering/OpenGL/ShaderProgram.hpp"
#include "Rendering/OpenGL/shaders.hpp"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <voxer/Filters/GradientFilter.hpp>

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

const EGLint pbuffer_attribs[] = {
    EGL_WIDTH, 1024, EGL_HEIGHT, 1024, EGL_NONE,
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
  vector<uint8_t> data;
  data.resize(tfcn_dim * 4);
  auto interploted = tfcn.interpolate();

  for (size_t i = 0; i < interploted.first.size(); i++) {
    auto &opacity = interploted.first[i];
    auto &color = interploted.second[i];
    data[i * 4 + 0] = static_cast<uint8_t>(color[0] * 255.0f);
    data[i * 4 + 1] = static_cast<uint8_t>(color[1] * 255.0f);
    data[i * 4 + 2] = static_cast<uint8_t>(color[2] * 255.0f);
    data[i * 4 + 3] = static_cast<uint8_t>(opacity * 255.0f);
  }

  glGenTextures(1, &tfcn_texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_1D, tfcn_texture);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, tfcn_dim, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, data.data());

  return tfcn_texture;
}

GLuint create_dataset_texture(voxer::StructuredGrid &dataset) {
  auto &info = dataset.info;
  auto &dimension = info.dimensions;
  auto &buffer = dataset.buffer;

  voxer::GradientFilter filter{};
  auto filtered = filter.process(dataset);

  GLuint volume_texture = 0;
  glGenTextures(1, &volume_texture);
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_3D, volume_texture);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, dimension[0], dimension[1],
               dimension[2], 0, GL_RGBA, GL_UNSIGNED_BYTE, filtered.data());

  return volume_texture;
}

float cube_vertices[] = {
    0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
};

unsigned int cube_vertex_indices[] = {0, 1, 2, 0, 2, 3, 0, 4, 1, 4, 5, 1,
                                      1, 5, 6, 6, 2, 1, 6, 7, 2, 7, 3, 2,
                                      7, 4, 3, 3, 4, 0, 4, 7, 6, 4, 6, 5};

float screen_quad_vertices[] = {
    -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};

} // namespace

namespace voxer {

OpenGLVolumeRenderer::OpenGLVolumeRenderer() {
  setup_context();
  setup_resources();
  setup_proxy_cude();
}

OpenGLVolumeRenderer::~OpenGLVolumeRenderer() noexcept {
  for (auto &item : m_dataset_cache) {
    auto id = item.second;
    glDeleteTextures(1, &id);
  }

  glDeleteBuffers(1, &m_VBO);
  glDeleteBuffers(1, &m_EBO);
  glDeleteVertexArrays(1, &m_VAO);
  glDeleteTextures(1, &m_entry_texture);
  glDeleteTextures(1, &m_exit_texture);
  glDeleteRenderbuffers(1, &m_RBO);
  glDeleteFramebuffers(1, &m_FBO);

  eglTerminate(m_egl_display);
}

void OpenGLVolumeRenderer::setup_context() {
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

void OpenGLVolumeRenderer::set_camera(const Camera &camera) {
  // update camera
  m_camera = camera;
  glViewport(0, 0, camera.width, camera.height);

  // delete old textures
  glDeleteTextures(1, &m_entry_texture);
  glDeleteTextures(1, &m_exit_texture);
  glDeleteRenderbuffers(1, &m_RBO);

  // create texture for ray entry info
  glGenTextures(1, &m_entry_texture);
  glActiveTexture(GL_TEXTURE0 + 3);
  glBindTexture(GL_TEXTURE_2D, m_entry_texture);
  glTextureStorage2D(m_entry_texture, 1, GL_RGBA32F, camera.width,
                     camera.height);
  glBindImageTexture(1, m_entry_texture, 0, GL_FALSE, 0, GL_READ_ONLY,
                     GL_RGBA32F);

  // create texture for ray exit info
  glGenTextures(1, &m_exit_texture);
  glActiveTexture(GL_TEXTURE0 + 4);
  glBindTexture(GL_TEXTURE_2D, m_exit_texture);
  glTextureStorage2D(m_exit_texture, 1, GL_RGBA32F, camera.width,
                     camera.height);
  glBindImageTexture(2, m_exit_texture, 0, GL_FALSE, 0, GL_READ_ONLY,
                     GL_RGBA32F);

  // create render buffer to store depth and stencil
  glGenRenderbuffers(1, &m_RBO);
  glBindRenderbuffer(GL_RENDERBUFFER, m_RBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, camera.width,
                        camera.height);

  // create a new frame buffer and attach resources
  glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, m_RBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_entry_texture, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         m_exit_texture, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    throw runtime_error("Framebuffer Object is not complete");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLVolumeRenderer::add_volume(const std::shared_ptr<Volume> &volume) {
  auto dataset = volume->dataset.get();

  if (m_dataset_cache.count(dataset) == 0) {
    auto texture = create_dataset_texture(*dataset);
    m_dataset_cache.emplace(dataset, texture);
  }

  m_volumes.emplace_back(volume);
}

void OpenGLVolumeRenderer::add_isosurface(
    const std::shared_ptr<voxer::Isosurface> &isosurface) {
  auto dataset = isosurface->dataset.get();

  if (m_dataset_cache.count(dataset) == 0) {
    auto texture = create_dataset_texture(*dataset);
    m_dataset_cache.emplace(dataset, texture);
  }

  m_isosurfaces.emplace_back(isosurface);
}

void OpenGLVolumeRenderer::render() {
  auto camera_to_target = glm::vec3(m_camera.target[0] - m_camera.pos[0],
                                    m_camera.target[1] - m_camera.pos[1],
                                    m_camera.target[2] - m_camera.pos[2]);
  auto distance = glm::length(camera_to_target);
  auto aspect =
      static_cast<float>(m_camera.width) / static_cast<float>(m_camera.height);
  auto projection = m_camera.type == Camera::Type::PERSPECTIVE
                        ? glm::perspective(45.0f, aspect, 0.1f, 10000.0f)
                        : glm::ortho(-distance, distance, -distance / aspect,
                                     distance / aspect, 0.1f, 5000.0f);
  auto view = glm::lookAt(
      glm::vec3(m_camera.pos[0], m_camera.pos[1], m_camera.pos[2]),
      glm::vec3(m_camera.target[0], m_camera.target[1], m_camera.target[2]),
      glm::vec3(m_camera.up[0], m_camera.up[1], m_camera.up[2]));

  m_raycast_program->use();
  m_raycast_program->setMat4("projection", projection);
  m_raycast_program->setMat4("view", view);
  m_raycast_program->setMat4("InverseView", glm::inverse(view));
  m_raycast_program->setVec3("cameraFront", camera_to_target[0],
                             camera_to_target[1], camera_to_target[2]);
  m_raycast_program->setInt("TF", 0);
  m_raycast_program->setInt("preIntTF", 1);
  m_raycast_program->setInt("Block", 2);
  m_raycast_program->setInt("entryPosTexture", 3);
  m_raycast_program->setInt("exitPosTexture", 4);
  m_raycast_program->setFloat("ka", 0.5f);
  m_raycast_program->setFloat("kd", 0.8f);
  m_raycast_program->setFloat("ks", 1.0f);
  m_raycast_program->setFloat("shininess", 100.0f);
  m_raycast_program->setVec3("lightDir", -1.0f, 0.0f, 0.0f);
  m_raycast_program->setVec3("bgColor", 0.0f, 0.0f, 0.0f);
  m_raycast_program->setBool("usePreIntTF", false);
  m_raycast_program->setBool("isPerspective",
                             m_camera.type == Camera::Type::PERSPECTIVE);
  m_raycast_program->setBool("gradPreCal", true);
  m_raycast_program->setBool("isMipMap", false);
  m_raycast_program->setInt("width", m_camera.width);
  m_raycast_program->setInt("height", m_camera.height);

  if (m_camera.pos[0] <= -0.1 || m_camera.pos[0] >= 1.1 ||
      m_camera.pos[1] <= -0.1 || m_camera.pos[1] >= 1.1 ||
      m_camera.pos[2] <= -0.1 || m_camera.pos[2] >= 1.1) {
    m_raycast_program->setBool("inner", false);
  } else {
    m_raycast_program->setBool("inner", true);
    m_raycast_program->setVec3("cameraPos", m_camera.pos);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (!m_isosurfaces.empty()) {
    m_raycast_program->setInt("drawMode", 2);
    for (auto &isosurface : m_isosurfaces) {
      auto dataset = isosurface->dataset.get();
      auto &dimensions = dataset->info.dimensions;
      auto max = static_cast<float>(
          std::max(dimensions[0], std::max(dimensions[1], dimensions[2])));

      auto model = glm::identity<glm::mat4>();
      model = glm::scale(
          model, glm::vec3(dimensions[0], dimensions[1], dimensions[2]));
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
      glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
      glDepthFunc(GL_LESS);

      // render pass 3
      m_raycast_program->use();
      m_raycast_program->setFloat("isoValue", isosurface->value / 255.0f);
      m_raycast_program->setVec3("isoColor", isosurface->color.data[0],
                                 isosurface->color.data[1],
                                 isosurface->color.data[2]);
      m_raycast_program->setMat4("model", model);
      m_raycast_program->setMat4("InverseModel", glm::inverse(model));
      m_raycast_program->setVec3("boundRatio", 1.0f / max * dimensions[0],
                                 1.0f / max * dimensions[1],
                                 1.0f / max * dimensions[2]);
      m_raycast_program->setInt("drawMode", 1);
      m_raycast_program->setVec3("boundRatio", 1.0f / max * dimensions[0],
                                 1.0f / max * dimensions[1],
                                 1.0f / max * dimensions[2]);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glDisable(GL_CULL_FACE);
      glBindVertexArray(m_VAO);
      glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    }
  }

  for (auto &volume : m_volumes) {
    auto dataset = volume->dataset.get();

    auto tfcn_texture = create_tfcn_texture(*volume->tfcn);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_1D, tfcn_texture);

    auto dataset_texture = m_dataset_cache.at(dataset);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_3D, dataset_texture);

    auto &dimensions = dataset->info.dimensions;
    auto max = static_cast<float>(
        std::max(dimensions[0], std::max(dimensions[1], dimensions[2])));

    auto model = glm::identity<glm::mat4>();
    model = glm::scale(model,
                       glm::vec3(dimensions[0], dimensions[1], dimensions[2]));
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
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    glDepthFunc(GL_LESS);

    // render pass 3
    m_raycast_program->use();
    m_raycast_program->setMat4("model", model);
    m_raycast_program->setMat4("InverseModel", glm::inverse(model));
    m_raycast_program->setVec3("boundRatio", 1.0f / max * dimensions[0],
                               1.0f / max * dimensions[1],
                               1.0f / max * dimensions[2]);
    m_raycast_program->setInt("drawMode", 2);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

    glDeleteTextures(1, &tfcn_texture);
  }

  glFlush();
}

auto OpenGLVolumeRenderer::get_colors() -> const Image & {
  m_image.width = m_camera.width;
  m_image.height = m_camera.height;
  m_image.channels = 3;
  m_image.data.resize(m_image.width * m_image.height * 3);

  // read pixels from the default frame buffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glReadPixels(0, 0, m_image.width, m_image.height, GL_RGB, GL_UNSIGNED_BYTE,
               reinterpret_cast<void *>(m_image.data.data()));

  return m_image;
}

void OpenGLVolumeRenderer::clear_scene() {
  // remove all primitives
  m_volumes.clear();
  m_isosurfaces.clear();
}

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

  glGenFramebuffers(1, &m_FBO);
}

void OpenGLVolumeRenderer::setup_proxy_cude() {
  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);

  glGenBuffers(1, &m_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, cube_vertices,
               GL_STATIC_DRAW);

  glGenBuffers(1, &m_EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_vertex_indices),
               cube_vertex_indices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
}

} // namespace voxer