//
// Created by wyz on 20-12-3.
//

#ifndef VOXER_VOLUMERAW2VIDEO_H
#define VOXER_VOLUMERAW2VIDEO_H
#include "VideoCapture.h"
#include <voxer/Data/StructuredGrid.hpp>
class VolumeRaw2Video {
public:
  VolumeRaw2Video(std::string raw_file_name);
  void SetupArgs(voxer::StructuredGrid::Axis axis, int32_t seconds_per_slice,
                 int32_t bit_rate);
  void Convert(std::string out_file_name);
  ~VolumeRaw2Video(){};

private:
  std::shared_ptr<voxer::StructuredGrid> volume;
  int32_t seconds_per_slice;
  int32_t bit_rate;
  uint32_t slice_w, slice_h, slice_depth;
  voxer::StructuredGrid::Axis axis;
  std::unique_ptr<VideoCapture> vc;
  std::string out_file_name;
};

#endif // VOXER_VOLUMERAW2VIDEO_H
