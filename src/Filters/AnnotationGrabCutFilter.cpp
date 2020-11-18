#include <opencv2/opencv.hpp>
#include <voxer/Filters/AnnotationGrabCutFilter.hpp>

namespace voxer {

auto AnnotationGrabCutFilter::process(
    const Image &image, const std::vector<Annotation> &annotations) const
    -> std::vector<Annotation> {
  if (annotations.empty()) {
    throw std::runtime_error("empty annotations");
  }

  auto &rect_annotation = annotations[0];
  if (rect_annotation.type != "Rect") {
    throw std::runtime_error("first annotation should be Rect type");
  }

  auto &left_top_point = rect_annotation.coordinates[0][0];
  auto &right_bottom_point = rect_annotation.coordinates[0][1];

  if (right_bottom_point[0] >= image.width ||
      right_bottom_point[1] >= image.height) {
    throw std::runtime_error("invalid annotation");
  }

  auto rect_width = right_bottom_point[0] - left_top_point[0];
  auto rect_height = right_bottom_point[1] - left_top_point[1];

  //  auto original_img = cv::Mat(rect_width, rect_height, CV_8UC1);
  //  for (size_t row = 0; row < rect_height; row++) {
  //    const uint8_t *src =
  //        image.data.data() + (row * image.width + left_top_point[0]) *
  //        sizeof(uint8_t);
  //    const uint8_t *dest =
  //        original_img.data + row * rect_width * sizeof(uint8_t);
  //    memcpy((void *)dest, src, rect_width * sizeof(uint8_t));
  //  }

  auto original_img = cv::Mat(image.width, image.height, CV_8UC1);
  memcpy(original_img.data, image.data.data(),
         image.data.size() * sizeof(uint8_t));

  int margin = 4;
  cv::Range rows(left_top_point[1] - margin, right_bottom_point[1] + margin);
  cv::Range cols(left_top_point[0] - margin, right_bottom_point[0] + margin);
  auto sub_image = original_img(rows, cols);

  auto colored_img = cv::Mat(sub_image.size(), CV_8UC3);
  cv::cvtColor(sub_image, colored_img, cv::COLOR_GRAY2RGB);

  auto rect = cv::Rect(margin, margin, rect_width, rect_height);
  cv::Mat mask = cv::Mat(colored_img.size(), CV_8UC1, cv::GC_BGD);
  (mask(rect)).setTo(cv::Scalar(cv::GC_PR_FGD));
  cv::Mat bgd_model;
  cv::Mat fgd_model;

  cv::grabCut(colored_img, mask, rect, bgd_model, fgd_model, iteration,
              cv::GC_INIT_WITH_RECT);

  cv::Mat bin_mask;
  bin_mask = mask & 1;

  std::vector<std::vector<cv::Point>> contours{};
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(bin_mask, contours, hierarchy, cv::RETR_TREE,
                   cv::CHAIN_APPROX_SIMPLE);

  std::vector<Annotation> result{};
  for (size_t i = 0; i < contours.size(); i++) {
    auto h = hierarchy[i];
    if (h[3] > 0) {
      // has parent
      continue;
    }

    auto area = cv::contourArea(contours[i]);
    if (area < threshold) {
      continue;
    }

    Annotation annotation{};
    annotation.id = rect_annotation.id;
    annotation.type = "Polygon";
    annotation.label = rect_annotation.label;
    annotation.coordinates.emplace_back();
    for (auto &point : contours[i]) {
      annotation.coordinates[0].emplace_back(Annotation::Point{
          static_cast<uint32_t>(point.x + left_top_point[0] - margin),
          static_cast<uint32_t>(point.y + left_top_point[1] - margin)});
    }
    result.emplace_back(std::move(annotation));
  }

  return result;
}

} // namespace voxer