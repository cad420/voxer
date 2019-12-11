#include <voxer/Image.hpp>
#define TJE_IMPLEMENTATION
#include "third_party/tiny_jpeg.h"
#include "utils/Debugger.hpp"
#include <cassert>
#include <chrono>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace voxer {

static Debugger debug("encoder");

static inline void _encode(void *context, void *data, int size) {
  auto buf = reinterpret_cast<vector<unsigned char> *>(context);
  auto res = reinterpret_cast<unsigned char *>(data);
  for (auto i = 0; i < size; i++) {
    buf->push_back(*(res + i));
  }
}

Image Image::encode(const uint8_t *data, uint32_t width, uint32_t height,
                    uint8_t channels, Image::Format format,
                    Image::Quality quality) {
  if (format != Image::Format::JPEG) {
    throw domain_error("format not suppported!");
  }

  auto start = chrono::steady_clock::now();

  Image image{width, height, channels, format};
  image.data.reserve(width * height * channels);

  auto quality_value = static_cast<int>(quality);
  assert(quality_value >= 0 && quality_value <= 3);
  tje_encode_with_func(_encode, &image.data, quality_value, width, height,
                       channels, data);

  const auto delta = chrono::duration_cast<chrono::milliseconds>(
      chrono::steady_clock::now() - start);
  debug.log(to_string(delta.count()) + " ms");

  return image;
}

Image Image::encode(const Image &image, Image::Format format,
                    Image::Quality quality) {
  if (image.format == Image::Format::JPEG) {
    return image;
  }

  return encode(image.data.data(), image.width, image.height, image.channels,
                format, quality);
}

} // namespace voxer
