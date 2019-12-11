#include "CommandParser.hpp"
#define CATCH_CONFIG_MAIN
#include "third_party/catch.hpp"
#include <iostream>

using namespace std;

TEST_CASE("CommandParser", "[parse]") {
  DatasetStore datasets;
  CommandParser parser;
  auto command = parser.parse(R"({"type":"query","params":{}})", 28, datasets);
  REQUIRE(command.type == Command::Type::Query);
  REQUIRE(get<std::nullptr_t>(command.params) == nullptr);
}