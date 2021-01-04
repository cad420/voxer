//
// Created by wyz on 20-12-3.
//

#include "VolumeRaw2Video.h"
VolumeRaw2Video::VolumeRaw2Video(std::string raw_file_name)
    : volume(voxer::StructuredGrid::Load(raw_file_name.c_str())) {}

void VolumeRaw2Video::SetupArgs(voxer::StructuredGrid::Axis axis,
                                int32_t seconds_per_slice, int32_t bit_rate) {
  this->out_file_name = out_file_name;
  this->axis = axis;
  this->seconds_per_slice = seconds_per_slice;
  this->bit_rate = bit_rate;
  auto dim = volume->info.dimensions;

  for (int i = 0; i < 3; i++) {
    if (dim[i] % 2 != 0) {
      dim[i] -= 1;
    }
  }

  switch (axis) {
  case voxer::StructuredGrid::Axis::X: {
    slice_w = dim[1];
    slice_h = dim[2];
    slice_depth = dim[0];
    break;
  }
  case voxer::StructuredGrid::Axis::Y: {
    slice_w = dim[0];
    slice_h = dim[2];
    slice_depth = dim[1];
    break;
  }
  case voxer::StructuredGrid::Axis::Z: {
    slice_w = dim[0];
    slice_h = dim[1];
    slice_depth = dim[2];
    break;
  }
  default:
    break;
  }
  vc.reset(
      Init(slice_w, slice_h, 1, bit_rate)); // fps can only pass integer number
}

void VolumeRaw2Video::Convert(std::string out_file_name) {
  for (size_t d = 0; d < slice_depth; d++) {
    auto image = volume->get_slice(axis, d);
    for (size_t i = 0; i < seconds_per_slice; i++) {
      AddFrame(image.data.data(), vc.get());
    }
  }
  Finish(vc.get(), out_file_name.c_str());
}
