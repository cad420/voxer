#include "DatasetStore.hpp"
#define CATCH_CONFIG_MAIN
#include "third_party/catch.hpp"
#include <iostream>

using namespace std;

TEST_CASE("DatasetStore", "[init, print]") {
  DatasetStore datasets;
  datasets.init(
      "/home/ukabuer/workspace/voxer/tests//data_configs/datasets_new.json");

  string res =
      R"("[{"name":"test-1","path":"1213123131"},{"name":"test-2","path":"121312313wqewq"}]")";

  REQUIRE(datasets.print() == res);
}