#pragma once
#include "DatasetManager.h"

typedef std::vector<unsigned char> Image;

enum class Axis {
  x, y, z
};

Image createSlice(const Dataset &dataset, const Axis axis, int index);