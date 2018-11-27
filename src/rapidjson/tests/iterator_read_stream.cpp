#include <mcpp/rapidjson/iterator_read_stream.hpp>

#include <string_view>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <catch2/catch.hpp>

namespace mcpp::rapidjson::tests {
namespace {

TEST_CASE("iterator_read_stream",
          "[mcpp][rapidjson][iterator_read_stream][stream]")
{
  std::string_view input("{\"foo\":\"bar\"}");
  iterator_read_stream stream(input.cbegin(),
                              input.cend());
  ::rapidjson::StringBuffer sb;
  ::rapidjson::Writer<::rapidjson::StringBuffer> writer(sb);
  ::rapidjson::Reader reader;
  auto result = reader.Parse(stream,
                             writer);
  REQUIRE(result);
  std::string_view sv(sb.GetString(),
                      sb.GetSize());
  CHECK(sv == input);
}

}
}
