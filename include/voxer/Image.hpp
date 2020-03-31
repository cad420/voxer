#pragma once
#include <cstdint>
#include <vector>

namespace voxer {

struct Image {
  enum class Format : uint8_t { RAW, JPEG };

  uint32_t width = 0;
  uint32_t height = 0;
  uint8_t channels = 0;
  Format format = Format::RAW;
  std::vector<unsigned char> data = {};

  enum class Quality : uint8_t { HIGH = 90, MEDIUM = 70, LOW = 50 };

  static Image encode(const uint8_t *data, uint32_t width, uint32_t height,
                      uint8_t channels, Image::Format format,
                      Image::Quality quality = Image::Quality::MEDIUM,
                      bool flip_vertically = true);

  static Image encode(const Image &image, Image::Format format,
                      Image::Quality Quality = Image::Quality::MEDIUM);
};

}; // namespace voxer