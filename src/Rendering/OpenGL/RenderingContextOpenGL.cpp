#include "Rendering/OpenGL/RenderingContextOpenGL.hpp"
#include <stdexcept>

using namespace std;

namespace voxer {

RenderingContextOpenGL::RenderingContextOpenGL() : width(800), height(600) {
  if (!glfwInit()) {
    throw runtime_error("create window failed");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(width, height, "voxer", nullptr, nullptr);

  glfwMakeContextCurrent(window);

  if (!gladLoadGL()) {
    throw runtime_error("failed to load gl");
  }

  glfwHideWindow(window);

  glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
}

RenderingContextOpenGL::~RenderingContextOpenGL() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void RenderingContextOpenGL::render(const Scene &scene,
                                    DatasetStore &datasets) {
  auto &camera = scene.camera;
  width = camera.width;
  height = camera.height;
  glViewport(0, 0, camera.width, camera.height);
  glClear(GL_COLOR_BUFFER_BIT);
  glfwSwapBuffers(window);
}

auto RenderingContextOpenGL::get_colors() -> const Image & {
  image.width = width;
  image.height = width;
  image.channels = 3;
  image.data.resize(width * height * 3);

  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE,
               reinterpret_cast<void *>(image.data.data()));

  return image;
}

} // namespace voxer