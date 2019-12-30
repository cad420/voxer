#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <voxer/Scene.hpp>
#include <voxer/scene/Camera.hpp>
#include <voxer/utils.hpp>
#define CATCH_CONFIG_MAIN
#include "third_party/catch.hpp"

using namespace std;
using namespace voxer;

TEST_CASE("nanoid", "[generate]") {
  auto collision = false;
  map<string, bool> table;
  for (size_t i = 0; i < 100000; i++) {
    auto res = nanoid();
    if (table.find(res) == table.end()) {
      table.emplace(res, true);
    } else {
      collision = true;
      break;
    }
  }

  REQUIRE(!collision);
}

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

TEST_CASE("json", "[deser, ser]") {
  voxer::Camera camera{};
  camera.width = 100;
  const voxer::Volume volume{};
  voxer::Scene scene{};
  scene.volumes.push_back(volume);
  scene.camera = camera;

  auto json = formatter::serialize(scene);
  {
    rapidjson::StringBuffer buffer{};
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);
    cout << buffer.GetString() << endl;
  }

  auto value = json.GetObject();
  auto it = value.FindMember("camera");
  auto &c = it->value;
  c["width"] = 233;

  formatter::deserialize(scene, json);
  {
    rapidjson::StringBuffer buffer{};
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);
    cout << buffer.GetString() << endl;
  }

  REQUIRE(std::is_class<voxer::SceneDataset>::value);
  REQUIRE((std::is_class<voxer::SceneDataset>::value &&
           !formatter::is_vector<voxer::SceneDataset>::value));
}