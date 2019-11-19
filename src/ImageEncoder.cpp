#include <voxer/ImageEncoder.hpp>
#define TJE_IMPLEMENTATION
#include "third_party/tiny_jpeg.h"
#include "utils/Debugger.hpp"
#include <string>
#include <vector>

using namespace std;
using ospcommon::vec2ui;

static Debugger debug("encoder");

namespace voxer {
void _encode(void *context, void *data, int size) {
  auto buf = (vector<unsigned char> *)context;
  auto res = (unsigned char *)data;
  for (auto i = 0; i < size; i++) {
    buf->push_back(*(res + i));
  }
}

vector<unsigned char> ImageEncoder::encode(unsigned char *data,
                                           ImageFormat format,
                                           uint32_t width,
                                           uint32_t height,
                                           uint8_t channels) {
  if (format != ImageFormat::JPEG) {
    throw string("format not suppported!");
  }

  auto start = chrono::steady_clock::now();

  vector<unsigned char> img;
  img.reserve(width * height * channels);
  auto quality = width == 64 ? 1 : 3;
  tje_encode_with_func(_encode, &img, quality, width, height, channels,
                       data);

  const auto delta = chrono::duration_cast<chrono::milliseconds>(
      chrono::steady_clock::now() - start);
  debug.log(to_string(delta.count()) + " ms");

  return img;
}
}
