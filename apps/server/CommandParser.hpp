#pragma once
#include <simdjson/jsonparser.h>
#include <string>

struct Command {
  enum class Type { Render, Save };

  Type type;
}

class CommandParser {
public:
  CommandParser();
  ~CommandParser();
  Command parse(const char *value, unsigned long long size);
  Command parse(const std::string &value);

private:
  simdjson::ParsedJson pj;
};
