#pragma once
#include <iostream>
#include <string>

namespace voxer {

enum struct LogLevel : int8_t {
  NOLOG = -2,
  ERROR = -1,
  WARNING = 0,
  INFO = 1,
  DEBUG = 2
};

struct Logger {
  static LogLevel level;
  std::string name;

  explicit Logger(const char *name) : name(name) {}

  void error(const std::string &content) {
    if (static_cast<uint8_t>(level) < static_cast<uint8_t>(LogLevel::ERROR)) {
      return;
    }
    std::cout << name << ": " << content << std::endl;
  }

  void warning(const std::string &content) {
    if (static_cast<uint8_t>(level) < static_cast<uint8_t>(LogLevel::WARNING)) {
      return;
    }
    std::cout << name << ": " << content << std::endl;
  }

  void info(const std::string &content) {
    if (static_cast<uint8_t>(level) < static_cast<uint8_t>(LogLevel::INFO)) {
      return;
    }
    std::cout << name << ": " << content << std::endl;
  }

  void debug(const std::string &content) {
    if (static_cast<uint8_t>(level) < static_cast<uint8_t>(LogLevel::DEBUG)) {
      return;
    }
    std::cout << name << ": " << content << std::endl;
  }
};

} // namespace voxer
