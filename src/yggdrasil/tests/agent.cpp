#include <mcpp/yggdrasil/agent.hpp>

#include <string>
#include <string_view>
#include <rapidjson/reader.h>
#include <rapidjson/stream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <catch2/catch.hpp>

namespace mcpp::yggdrasil::tests {
namespace {

TEST_CASE("to_json (agent)",
          "[mcpp][yggdrasil][agent][to_json][json]")
{
  ::rapidjson::StringBuffer sb;
  ::rapidjson::Writer<::rapidjson::StringBuffer> w(sb);
  agent a;
  a.name = "foo";
  a.version = 15;
  auto result = to_json(a,
                        w);
  REQUIRE(result);
  std::string_view str(sb.GetString(),
                       sb.GetSize());
  std::string_view expected("{\"name\":\"foo\","
                             "\"version\":15}");
  CHECK(str == expected);
}

TEST_CASE("agent_parser",
          "[mcpp][yggdrasil][agent][json][parse]")
{
  agent a;
  agent_parser handler(a);
  ::rapidjson::Reader reader;
  SECTION("Empty object") {
    ::rapidjson::StringStream ss("{}");
    auto result = reader.Parse(ss,
                               handler);
    CHECK_FALSE(result);
    auto ec = handler.error_code();
    CHECK(ec);
  }
  SECTION("Not an object") {
    ::rapidjson::StringStream ss("5");
    auto result = reader.Parse(ss,
                               handler);
    CHECK_FALSE(result);
    auto ec = handler.error_code();
    CHECK(ec);
  }
  SECTION("Good") {
    ::rapidjson::StringStream ss("{\"name\":\"foo\","
                                  "\"version\":15}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(a.name == "foo");
    CHECK(a.version == 15);
  }
}

}
}
