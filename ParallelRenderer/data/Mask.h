#pragma once
#include <string>
#include <ospray/ospray_cpp.h>

std::string createDatasetByMask(std::string first, std::string second, ospcommon::vec2f &rangeFirst, ospcommon::vec2f &rangeSecond);
