#include <opencv2/opencv.hpp>
#include <voxer/Filters/AnnotationLevelset.hpp>
#include <voxer/IO/MRCReader.hpp>

using namespace voxer;

int main(int argc, const char **argv) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " /path/to/mrc/file\n";
    return 0;
  }

  MRCReader reader(argv[1]);
  auto dataset = reader.load();

  auto slice = dataset->get_slice(StructuredGrid::Axis::Z, 1);

  Annotation annotation{};
  annotation.type = "Polygon";
  annotation.id = 1;
  annotation.label = "1";
  annotation.bbox = {
      {212, 1301, 367, 1469},
  };
  annotation.coordinates.emplace_back();
  std::array<uint32_t, 2> cols = {212, 367};
  std::array<uint32_t, 2> rows = {1301, 1469};
  for (auto y = rows[0]; y < rows[1]; y++) {
    for (auto x = cols[0]; x < cols[1]; x++) {
      annotation.coordinates[0].emplace_back(Annotation::Point{x, y});
    }
  }

  AnnotationLevelSetFilter filter{};
  auto result = filter.process(annotation, slice);

  std::vector<cv::Point> contour{};
  for (auto &item : result.coordinates[0]) {
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