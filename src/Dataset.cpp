#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <seria/serialize.hpp>
#include <voxer/Dataset.hpp>

using namespace std;

namespace seria {

template <> inline auto registerObject<voxer::Dataset>() {
  using Dataset = voxer::Dataset;
  return std::make_tuple(member("name", &Dataset::name),
                         member("variable", &Dataset::variable),
                         member("timestep", &Dataset::timestep),
                         member("histogram", &Dataset::histogram));
}

} // namespace seria

namespace voxer {

string Dataset::serialize() const {
  auto json = seria::serialize(*this);
  rapidjson::StringBuffer rapidjson_buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(rapidjson_buffer);
  json.Accept(writer);
  return rapidjson_buffer.GetString();
}

} // namespace voxer