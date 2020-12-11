#include <catch2/catch_all.hpp>
#include <voxer/IO/MRCReader.hpp>

TEST_CASE("MRCReader", "[read]") {
  voxer::MRCReader reader {"BBb.mrc"};
  auto dataset = reader.load();
}