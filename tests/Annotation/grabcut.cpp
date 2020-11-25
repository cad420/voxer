#include <opencv2/opencv.hpp>
#include <voxer/Filters/AnnotationGrabCutFilter.hpp>
#include <voxer/IO/MRCReader.hpp>

using namespace voxer;

int main(int argc, const char **argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " /path/to/mrc/file\n";
    return 0;
  }

  MRCReader reader(argv[1]);
  auto dataset = reader.load();

  auto slice = dataset->get_slice(StructuredGrid::Axis::Z, 0);

  Annotation annotation{};
  annotation.type = "Rect";
  annotation.id = 1;
  annotation.label = "1";
  annotation.coordinates.emplace_back();
  annotation.coordinates[0].emplace_back(Annotation::Point{212, 1301});
  annotation.coordinates[0].emplace_back(Annotation::Point{367, 1469});

  std::vector<Annotation> annotations{};
  annotations.emplace_back(annotation);

  AnnotationGrabCutFilter filter{};
  auto result = filter.process(slice, annotations);

  std::vector<cv::Point> contour{};
  for (auto &item : result[0].coordinates[0]) {
    contour.emplace_back(
        cv::Point{static_cast<int>(item[0]), static_cast<int>(item[1])});
  }
  std::vector<std::vector<cv::Point>> contours{};
  contours.emplace_back(contour);

  auto img = cv::Mat(slice.width, slice.height, CV_8UC1);
  memcpy(img.data, slice.data.data(), slice.data.size() * sizeof(uint8_t));
  cv::drawContours(img, contours, 0, cv::Scalar(255), cv::FILLED);

  auto should_exit = false;
  for (;;) {
    if (should_exit) {
      break;
    }

    char c = (char)cv::waitKey(0);
    switch (c) {
    case '\x1b':
      should_exit = true;
      break;
    default:
      cv::imshow("grabcut", img);
      break;
    }
  }

  return 0;
}