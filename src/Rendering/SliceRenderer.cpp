#include <voxer/Rendering/SliceRenderer.hpp>
#include <voxer/Data/StructuredGrid.hpp>
#include <stdexcept>

using namespace std;

namespace voxer {

void SliceRenderer::set_dataset(StructuredGrid *dataset) {
  if (dataset == nullptr) {
    throw runtime_error("Invalid dataset");
  }
  m_dataset = dataset;
  m_info = dataset->info;
}

void SliceRenderer::add_mark(StructuredGrid *dataset, const string &hex_color) {
  auto color = hex_color_to_float(hex_color);
  this->add_mark(dataset, color);
}

void SliceRenderer::add_mark(StructuredGrid *dataset, const array<float, 3> &color) {
  if (dataset == nullptr) {
    throw runtime_error("Invalid mark dataset");
  }

  if (m_dataset == nullptr) {
    throw runtime_error("Should set dataset first");
  }

  if (m_info != dataset->info) {
    throw runtime_error(
        "Dataset and mark have different dimensions or data types");
  }

  this->m_marks.emplace_back(make_pair(dataset, color));
}

static void draw_mark(StructuredGrid *dataset,
                      const vector<SliceRenderer::MarkType> &marks,
                      uint32_t voxel_idx, uint32_t pixel_idx, Image &target) {
  auto should_draw_dataset = true;

  for (auto &mark : marks) {
    auto mark_dataset = mark.first;
    auto color = mark.second;
    if (mark_dataset->buffer[voxel_idx] != 0) {
      target.data[pixel_idx] = color[0] * 255;
      target.data[pixel_idx + 1] = color[1] * 255;
      target.data[pixel_idx + 2] = color[2] * 255;
      should_draw_dataset = false;
    }
  }

  if (!should_draw_dataset) {
    return;
  }

  target.data[pixel_idx] = dataset->buffer[voxel_idx];
  target.data[pixel_idx + 1] = dataset->buffer[voxel_idx];
  target.data[pixel_idx + 2] = dataset->buffer[voxel_idx];
}

Image SliceRenderer::render(StructuredGrid::Axis axis, uint32_t slice) const {
  Image result{};
  result.channels = 3;

  if (m_dataset == nullptr) {
    return result;
  }

  switch (axis) {
  case StructuredGrid::Axis::X: {
    if (slice >= m_info.dimensions[0]) {
      throw runtime_error("slice idx overflow");
    }
    result.width = m_info.dimensions[1];
    result.height = m_info.dimensions[2];
    result.data.resize(result.width * result.height * result.channels *
                       sizeof(uint8_t));
    for (int z = 0; z < m_info.dimensions[2]; z++) {
      for (int y = 0; y < m_info.dimensions[1]; y++) {
        auto voxel_idx = slice + y * m_info.dimensions[0] +
                         z * m_info.dimensions[0] * m_info.dimensions[1];
        auto pixel_idx = (y + z * result.width) * result.channels;
        draw_mark(m_dataset, m_marks, voxel_idx, pixel_idx, result);
      }
    }
    break;
  }
  case StructuredGrid::Axis::Y: {
    if (slice >= m_info.dimensions[1]) {
      throw runtime_error("slice idx overflow");
    }
    result.width = m_info.dimensions[0];
    result.height = m_info.dimensions[2];
    result.data.reserve(result.width * result.height * result.channels *
                        sizeof(uint8_t));
    for (int z = 0; z < m_info.dimensions[2]; z++) {
      for (int x = 0; x < m_info.dimensions[0]; x++) {
        auto voxel_idx = x + slice * m_info.dimensions[0] +
                         z * m_info.dimensions[0] * m_info.dimensions[1];
        auto pixel_idx = (x + z * result.width) * result.channels;
        draw_mark(m_dataset, m_marks, voxel_idx, pixel_idx, result);
      }
    }
    break;
  }
  case StructuredGrid::Axis::Z: {
    if (slice >= m_info.dimensions[2]) {
      throw runtime_error("slice idx overflow");
    }
    result.width = m_info.dimensions[0];
    result.height = m_info.dimensions[1];
    result.data.reserve(result.width * result.height * result.channels *
                        sizeof(uint8_t));
    for (int y = 0; y < m_info.dimensions[1]; y++) {
      for (int x = 0; x < m_info.dimensions[0]; x++) {
        auto voxel_idx = x + y * m_info.dimensions[0] +
                         slice * m_info.dimensions[0] * m_info.dimensions[1];
        auto pixel_idx = (x + y * result.width) * result.channels;
        draw_mark(m_dataset, m_marks, voxel_idx, pixel_idx, result);
      }
    }
    break;
  }
  default:
    break;
  }

  return result;
}

} // namespace voxer