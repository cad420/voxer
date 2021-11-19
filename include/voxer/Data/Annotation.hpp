#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <opencv2/opencv.hpp>

namespace voxer {

struct StructuredGrid;


struct Annotation {
  enum struct Type { Polygon, Rect };
  using Point = std::array<uint32_t, 2>;
  // using Point = cv::Point;
  using Bbox = std::array<int32_t, 4>;

  Bbox bbox;
  std::string type;
  uint32_t id = 0;
  std::string label;
  uint32_t slice;
  std::vector<std::vector<Point>> coordinates;

  Annotation() = default;
  Annotation(Bbox& _bbox,
  std::string& _type,uint32_t _id,
  std::string& _label,uint32_t _slice,
  std::vector<std::vector<Point>>& _coordinates): bbox(_bbox),type(_type),id(_id),label(_label),slice(_slice),coordinates(std::move<decltype(_coordinates)>(_coordinates))
  {
      
  }

  bool has_hole_inside();
};

std::shared_ptr<std::vector<Annotation>> getAnnotations(std::shared_ptr<StructuredGrid>,std::map<uint8_t,std::string>&);

} // namespace voxer
