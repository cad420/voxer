#pragma once
#include <voxer/Data/StructuredGrid.hpp>

namespace voxer {

struct BlurFilter {
  virtual void process(StructuredGrid *volume_data) = 0;
  virtual void process(Image *image) = 0;
};

struct MedianBlurFilter : BlurFilter {
  float m_value = 5.0f;

  void process(StructuredGrid *volume_data) override;
  void process(Image *image) override;
};

struct GuassianBlurFilter : BlurFilter {
  float sig_x = 2;
  float sig_y = 2;
  float sig_z = 1;

  void process(StructuredGrid *volume_data) override;
  void process(Image *image) override;
};

struct BilateralBlurFilter : BlurFilter {
  float sig_x = 2;
  float sig_y = 2;
  float sig_z = 1;
  float sig_r = 10;

  void process(StructuredGrid *volume_data) override;
  void process(Image *image) override;
};

} // namespace voxer
