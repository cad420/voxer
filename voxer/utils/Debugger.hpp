#pragma once
#include <iostream>
#include <string>

struct Debugger {
  std::string name;
  Debugger(std::string n) : name(n) {}
  void log(std::string info);
};
