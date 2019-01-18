#include <alloca.h>
#include <cstdio>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string>

#include "ospray/ospray_cpp.h"

#include <fstream>
#include <vector>

using namespace std;
using namespace ospcommon;
using namespace ospray::cpp;

// helper function to write the rendered image as PPM file
void writePPM(const char *fileName, const ospcommon::vec2i &size,
              const uint32_t *pixel) {
  FILE *file = fopen(fileName, "wb");
  if (file == nullptr) {
    fprintf(stderr, "fopen('%s', 'wb') failed: %d", fileName, errno);
    return;
  }
  fprintf(file, "P6\n%i %i\n255\n", size.x, size.y);
  unsigned char *out = (unsigned char *)alloca(3 * size.x);
  for (int y = 0; y < size.y; y++) {
    const unsigned char *in =
        (const unsigned char *)&pixel[(size.y - 1 - y) * size.x];
    for (int x = 0; x < size.x; x++) {
      out[3 * x + 0] = in[4 * x + 0];
      out[3 * x + 1] = in[4 * x + 1];
      out[3 * x + 2] = in[4 * x + 2];
    }
    fwrite(out, 3 * size.x, sizeof(char), file);
  }
  fprintf(file, "\n");
  fclose(file);
}

int main(int argc, const char **argv) {
  // image size
  ospcommon::vec2i imgSize;
  imgSize.x = 1024; // width
  imgSize.y = 768;  // height

  // camera
  ospcommon::vec3f cam_pos{0.f, 0.f, 300.f};
  ospcommon::vec3f cam_up{0.f, 1.f, 0.f};
  ospcommon::vec3f cam_view{0.0f, 0.f, -1.f};

  auto file = fopen("/run/media/ukabuer/B6A8919FA8915F25/"
                    "csafe-heptane-302-volume/csafe-heptane-302-volume.raw",
                    "rb");

  vec3i dimensions(302, 302, 302);
  int voxelSize = sizeof(unsigned char);
  vector<unsigned char> buffer(dimensions.x * dimensions.y * dimensions.z);
  size_t read =
      fread(buffer.data(), 1, dimensions.x * dimensions.y * dimensions.z, file);

  OSPError init_error = ospInit(&argc, argv);
  if (init_error != OSP_NO_ERROR)
    return init_error;

  Camera camera("perspective");
  camera.set("aspect", imgSize.x / (float)imgSize.y);
  camera.set("pos", cam_pos);
  camera.set("dir", cam_view);
  camera.set("up", cam_up);
  camera.commit();

  vector<vec3f> colors = {vec3f(0, 0, 0.56),  vec3f(0, 0, 1), vec3f(0, 1, 1),
                          vec3f(0.5, 1, 0.5), vec3f(1, 1, 0), vec3f(1, 0, 0),
                          vec3f(0.5, 0, 0)};
  Data colorsData(colors.size(), OSP_UCHAR, colors.data(),
                  OSP_DATA_SHARED_BUFFER);
  colorsData.commit();

  vector<float> opacities = {0.001, 0.8};
  Data opacityData(opacities.size(), OSP_UCHAR, opacities.data(),
                   OSP_DATA_SHARED_BUFFER);
  opacityData.commit();

  TransferFunction tfcn("piecewise_linear");
  tfcn.set("colors", colorsData);
  tfcn.set("opacities", opacityData);
  tfcn.set("valueRange", vec2i(0, 255));
  tfcn.commit();

  Data data(buffer.size(), OSP_UCHAR, buffer.data(), OSP_DATA_SHARED_BUFFER);
  data.commit();

  Volume volume("shared_structured_volume");
  volume.set("voxelType", "uchar");
  volume.set("dimensions", dimensions);
  volume.set("voxelData", data);
  volume.set("transferFunction", tfcn);
  volume.set("gridOrigin", vec3f(-dimensions / 2));
  cout << -dimensions / 2 << endl;
  volume.commit();

  Model world, model;
  model.addVolume(volume);

  vector<unsigned char> values = { 5 };
  ospray::cpp::Geometry isosurface("isosurfaces");
  ospray::cpp::Data valuesData(values.size(), OSP_UCHAR,
                         values.data());
  isosurface.set("isovalues", valuesData);
  isosurface.set("volume", volume);
  isosurface.commit();
  model.addGeometry(isosurface);

  model.commit();

  // affine3f initial(one);
  // auto transform = affine3f::scale(vec3f(0.1, 0.1, 1));
  // auto ins = model.createInstance(transform);
  // world.addGeometry(ins);
  // world.commit();

  // create renderer

  // create and setup light for Ambient Occlusion
  ospray::cpp::Light light("ambient");
  light.commit();
  auto lightHandle = light.handle();
  ospray::cpp::Data lights(1, OSP_LIGHT, &lightHandle);
  lights.commit();

  ospray::cpp::Renderer renderer("scivis");
  renderer.set("aoSamples", 0);
  renderer.set("bgColor", 1.0f);
  renderer.set("model", model);
  renderer.set("camera", camera);
  renderer.set("lights", lights);
  renderer.commit();

  // create and setup framebuffer
  ospray::cpp::FrameBuffer framebuffer(
      imgSize, OSP_FB_SRGBA, OSP_FB_COLOR | /*OSP_FB_DEPTH |*/ OSP_FB_ACCUM);
  framebuffer.clear(OSP_FB_COLOR | OSP_FB_ACCUM);

  // render 10 more frames, which are accumulated to result in a better
  // converged image
  for (int frames = 0; frames < 10; frames++)
    renderer.renderFrame(framebuffer, OSP_FB_COLOR | OSP_FB_ACCUM);

  uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
  fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
  writePPM("accumulatedFrameCpp.ppm", imgSize, fb);
  framebuffer.unmap(fb);

  // final cleanups
  renderer.release();
  camera.release();
  lights.release();
  light.release();
  framebuffer.release();
  world.release();

  ospShutdown();

  return 0;
}