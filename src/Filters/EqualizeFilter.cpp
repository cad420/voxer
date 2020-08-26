#define cimg_use_openmp 1
#include <third_party/CImg.h>
#include <voxer/Filters/EqualizeFilter.hpp>

using namespace std;

namespace voxer {

void EqualizeFilter::process(Image *image) const {
  cimg_library::CImg<uint8_t> cimage(image->data.data(), image->width,
                                     image->height, 1, image->channels, true);
  cimage.equalize(n, min, max);
}

void EqualizeFilter::process(StructuredGrid *volume_data) const {
  auto &dimensions = volume_data->info.dimensions;
  cimg_library::CImg<uint8_t> cimage(volume_data->buffer.data(), dimensions[0],
                                     dimensions[1], dimensions[2], 1, true);
  cimage.equalize(n, min, max);
}

} // namespace voxer