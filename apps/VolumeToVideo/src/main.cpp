#include "VolumeRaw2Video.h"
#include <iostream>
#include <voxer/Data/StructuredGrid.hpp>

inline auto get_filename(const std::string &filepath) -> std::string {
  auto ext_idx = filepath.find_last_of('.');
  if (ext_idx == std::string::npos) {
    return "";
  }

  return filepath.substr(0, ext_idx);
}

int main(int argc, const char **args) {
  std::string filepath = args[1];
  auto filename = get_filename(filepath);

  auto seconds_per_slice = 1;
  auto x_bit_rate = 80;
  auto y_bit_rate = 80;
  auto z_bit_rate = 800;

  VolumeRaw2Video cvt_x(filepath);
  cvt_x.SetupArgs(voxer::StructuredGrid::Axis::X, seconds_per_slice,
                  x_bit_rate);
  cvt_x.Convert(filename + "-" + std::to_string(x_bit_rate) + "-x.mp4");

  VolumeRaw2Video cvt_y(filepath);
  cvt_y.SetupArgs(voxer::StructuredGrid::Axis::Y, seconds_per_slice,
                  y_bit_rate);
  cvt_y.Convert(filename + "-" + std::to_string(y_bit_rate) + "-y.mp4");

  VolumeRaw2Video cvt_z(filepath);
  cvt_z.SetupArgs(voxer::StructuredGrid::Axis::Z, seconds_per_slice,
                  z_bit_rate);
  cvt_z.Convert(filename + "-" + std::to_string(z_bit_rate) + "-z.mp4");
}