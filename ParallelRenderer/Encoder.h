#pragma once
#include "ospray/ospcommon/vec.h"
#include <vector>
#include <string>

class Encoder {
  public:
  std::vector<unsigned char> encode(std::vector<unsigned char> &data, ospcommon::vec2ui size, std::string format);
};