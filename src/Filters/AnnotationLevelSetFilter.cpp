#include "seg_levelset.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <voxer/Filters/AnnotationLevelset.hpp>

using namespace std;

namespace voxer {

Annotation AnnotationLevelSetFilter::process(const Annotation &annotation,
                                             const Image &image) const {
  assert(image.channels == 1);

  auto originalImg = cv::Mat(image.width, image.height, CV_8UC1);
  std::vector<cv::Point> contour{};
  if (annotation.coordinates.empty()) {
    return {};
  }

  for (auto &item : annotation.coordinates[0]) {
    contour.emplace_back(
        cv::Point{static_cast<int>(item[0]), static_cast<int>(item[1])});
  }

  // 先把图像转换为 float，方便传入 mask
  memcpy(originalImg.data, image.data.data(),
         image.data.size() * sizeof(uint8_t));
  auto candidateImg = cv::Mat(originalImg.size(), CV_64FC1);
  originalImg.convertTo(candidateImg, CV_64FC1);

  // 将 contour 转换为前景为 -2，背景为 2 的 mask
  cv::Mat candidateMask = cv::Mat::ones(originalImg.size(), CV_64FC1) * 2;
  fillPoly(candidateMask, vector<vector<cv::Point>>{contour}, -2);

  // 运行 Level Set 算法
  double range[3] = {0, 0, 0};
  cv::Mat levelSetMask = segSlice(candidateMask, candidateImg, cv::Mat(), range,
                                  inner_iteration, outer_iteration);

  levelSetMask.convertTo(levelSetMask, CV_8UC1);

  vector<vector<cv::Point>> contours;
  cv::findContours(levelSetMask, contours, cv::RETR_TREE,
                   cv::CHAIN_APPROX_SIMPLE);

  // Level Set 可能会返回多个等值面，只返回最大的那个
  auto maxArea = 0.0;
  std::vector<cv::Point> maxContour;
  for (auto &item : contours) {
    auto area = contourArea(item);
    // cout << "area: " << area << endl;
    if (area > maxArea) {
      maxArea = area;
      maxContour = contour;
    }
  }

  Annotation result{};
  result.id = annotation.id;
  result.type = annotation.type;
  result.label = annotation.label;
  result.coordinates.emplace_back();
  for (auto &item : maxContour) {
    result.coordinates[0].emplace_back(Annotation::Point{
        static_cast<uint32_t>(item.x), static_cast<uint32_t>(item.y)});
  }

  return result;
}

} // namespace voxer