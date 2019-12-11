#include "utils.hpp"
#define CATCH_CONFIG_MAIN
#include "third_party/catch.hpp"

using namespace std;

TEST_CASE("hex_color_to_float", "[conversion]") {

  const string v1 = "#ffffff";
  auto res1 = hex_color_to_float(v1);
  REQUIRE((res1[0] == 1.0f && res1[1] == 1.0f && res1[2] == 1.0f));

  const string v2 = "#ffff00";
  auto res2 = hex_color_to_float(v2);
  REQUIRE((res2[0] == 1.0f && res2[1] == 1.0f && res2[2] == 0.0f));

  const string v3 = "#00ff00";
  auto res3 = hex_color_to_float(v3);
  REQUIRE((res3[0] == 0.0f && res3[1] == 1.0f && res3[2] == 0.0f));
}