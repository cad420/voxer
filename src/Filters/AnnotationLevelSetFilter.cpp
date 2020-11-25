#include "seg_levelset.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <voxer/Filters/AnnotationLevelset.hpp>

using namespace std;

namespace voxer {

Annotation AnnotationLevelSetFilter::process(const Annotation &annotation,
                                             const Image &image) const {
  assert(image.channels == 1);

  if (annotation.coordinates.empty()) {
    return {};
  }

  // 先把图像转换为 float，方便传入 mask
  auto original_img = cv::Mat(image.width, image.height, CV_8UC1);
  memcpy(original_img.data, image.data.data(),
         image.data.size() * sizeof(uint8_t));

  auto &bbox = annotation.bbox;
  cv::Range rows(int((3 * bbox[1] - bbox[3]) / 2),
                 int((3 * bbox[3] - bbox[1]) / 2));
  cv::Range cols(int((3 * bbox[0] - bbox[2]) / 2),
                 int((3 * bbox[2] - bbox[0]) / 2));
  auto sub_image = original_img(rows, cols);

  std::vector<cv::Point> contour{};
  for (auto &item : annotation.coordinates[0]) {
    contour.emplace_back(
        cv::Point{static_cast<int>(item[0] - cols.start),
                  static_cast<int>(item[1] - rows.start)});
  }

  auto candidate_img = cv::Mat(sub_image.size(), CV_64FC1);
  sub_image.convertTo(candidate_img, CV_64FC1);

  // 将 contour 转换为前景为 -2，背景为 2 的 mask
  cv::Mat candidate_mask = cv::Mat::ones(sub_image.size(), CV_64FC1) * 2;
  fillPoly(candidate_mask, vector<vector<cv::Point>>{contour}, -2);

  // 运行 Level Set 算法
  double range[3] = {0, 0, 0};
  cv::Mat levelset_mask = segSlice(candidate_mask, candidate_img, cv::Mat(),
                                   range, inner_iteration, outer_iteration);

  levelset_mask.convertTo(levelset_mask, CV_8UC1);

  vector<vector<cv::Point>> contours;
  cv::findContours(levelset_mask, contours, cv::RETR_TREE,
                   cv::CHAIN_APPROX_SIMPLE);

  // Level Set 可能会返回多个等值面，只返回最大的那个
  auto maxArea = 0.0;
  std::vector<cv::Point> max_contour;
  for (auto &item : contours) {
    auto area = contourArea(item);
    if (area > maxArea) {
      maxArea = area;
      max_contour = item;
    }
  }

  Annotation result{};
  result.id = annotation.id;
  result.type = annotation.type;
  result.label = annotation.label;
  result.coordinates.emplace_back();
  for (auto &item : max_contour) {
    result.coordinates[0].emplace_back(
        Annotation::Point{static_cast<uint32_t>(item.x + cols.start),
                          static_cast<uint32_t>(item.y + rows.start)});
  }

  return result;
}

} // namespace voxer