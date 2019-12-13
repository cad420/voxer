#include "CommandParser.hpp"
#define CATCH_CONFIG_MAIN
#include "third_party/catch.hpp"
#include <iostream>
#include <voxer/DatasetStore.hpp>

using namespace std;

TEST_CASE("CommandParser", "[parse]") {
  string json = R"({"type":"query","params":{}})";

  voxer::DatasetStore datasets;
  CommandParser parser;
  auto command = parser.parse(json);
  REQUIRE(command.type == Command::Type::Query);
  REQUIRE(get<std::nullptr_t>(command.params) == nullptr);
}