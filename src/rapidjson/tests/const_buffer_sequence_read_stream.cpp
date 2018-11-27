#include <mcpp/rapidjson/const_buffer_sequence_read_stream.hpp>

#include <string>
#include <string_view>
#include <boost/asio/buffer.hpp>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <catch2/catch.hpp>

namespace mcpp::rapidjson::tests {
namespace {

TEST_CASE("const_buffer_sequence_read_stream",
          "[mcpp][rapidjson][const_buffer_sequence_read_stream][stream]")
{
  std::string input("{\"foo\":\"bar\"}");
  const_buffer_sequence_read_stream stream(boost::asio::buffer(input));
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
