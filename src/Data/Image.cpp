#include <voxer/Data/Image.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third_party/stb_image_write.h"
#include <cassert>
#include <chrono>
#include <stdexcept>
#include <vector>

using namespace std;

namespace voxer {

namespace {

void stbi_write_func(void *context, void *data, int size) {
  auto image = reinterpret_cast<vector<uint8_t> *>(context);
  auto encoded = reinterpret_cast<uint8_t *>(data);
  for (int i = 0; i < size; i++) {
    image->push_back(encoded[i]);
  }
}

} // namespace

Image Image::encode(const uint8_t *data, uint32_t width, uint32_t height,
                    uint8_t channels, Image::Format format,
                    Image::Quality quality, bool flip_vertically) {
  if (format != Image::Format::JPEG) {
    throw domain_error("currently only support JPEG format!");
  }

  auto start = chrono::steady_clock::now();

  Image image{width, height, channels, format};
  image.data.reserve(width * height * channels);

  auto quality_value = static_cast<int>(quality);
  assert(quality_value >= 0 && quality_value <= 100);

  if (flip_vertically) {
    stbi_flip_vertically_on_write(1);
  } else {
    stbi_flip_vertically_on_write(0);
  }

  auto res = stbi_write_jpg_to_func(
      stbi_write_func, reinterpret_cast<void *>(&image.data), width, height,
      channels, data, quality_value);

  if (res == 0) {
    throw runtime_error("encoding image failed.");
  }

  return image;
}

Image Image::encode(const Image &image, Image::Format format,
                    Image::Quality quality, bool flip_vertically) {
  if (image.format == format) {
    return image;
  }

  return encode(image.data.data(), image.width, image.height, image.channels,
                format, quality, flip_vertically);
}

auto Image::at(uint32_t row, uint32_t col) const -> uint8_t {
  assert(row < height && col < width);
  return data[row * width + col];
}

} // namespace voxer
