#include <CImg.h>
#include <voxer/Filters/ContrastFilter.hpp>

using namespace std;

namespace voxer {

unique_ptr<StructuredGrid>
ContrastFilter::process(StructuredGrid *volume_data) const {
  auto &info = volume_data->info;
  cimg_library::CImg<uint8_t> cimage(volume_data->buffer.data(),
                                     info.dimensions[0], info.dimensions[1],
                                     info.dimensions[2], 1, true);

  auto res = make_unique<StructuredGrid>();
  res->buffer.resize(volume_data->buffer.size());

  auto mean = cimage.mean();

  //#pragma omp parallel for
  for (size_t index = 0; index < res->buffer.size(); ++index) {
    auto t = volume_data->buffer[index] - mean;
    t *= contrast;          // Adjust contrast
    t += mean * brightness; // Adjust brightness
    res->buffer[index] = (t > 255.0) ? 255 : (t < 0.0 ? (0) : (t));
  }

  return res;
}

} // namespace voxer