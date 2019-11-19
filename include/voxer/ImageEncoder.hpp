#pragma once
#include <ospray/ospcommon/vec.h>
#include <string>
#include <vector>

namespace voxer {

using Image = std::vector<unsigned char>
enum class ImageFormat {
   JPEG
};

class ImageEncoder {
public:
   Image encode(unsigned char *data,
                                    ImageFormat format,
                                    uint32_t width,
                                    uint32_t height,
                                    uint8_t channels);
};

}
