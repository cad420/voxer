#include "CommandParser.hpp"
#include <iostream>

void CommandParser::parse(const std::string &value) {
  simdjson::ParsedJson pj;
  const int res = simdjson::josn_parser(value, pj);
  if (res != 0) {
    std::cout << "Error parsing: " << simdjson::error_message(res) << std::endl;
  }
}