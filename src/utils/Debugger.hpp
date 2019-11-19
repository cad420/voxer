#pragma once
#include <iostream>
#include <string>

namespace voxer {
struct Debugger {
  std::string name;
  Debugger(std::string n) : name(n) {}
  void log(std::string info);
};
}
