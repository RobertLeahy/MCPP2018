#include <mcpp/yggdrasil/property.hpp>

#include <string>
#include <string_view>
#include <rapidjson/reader.h>
#include <rapidjson/stream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <catch2/catch.hpp>

namespace mcpp::yggdrasil::tests {
namespace {

TEST_CASE("to_json (property)",
          "[mcpp][yggdrasil][property][to_json][json]")
{
  ::rapidjson::StringBuffer sb;
  ::rapidjson::Writer<::rapidjson::StringBuffer> w(sb);
  property p;
  p.name = "foo";
  p.value = "bar";
  auto result = to_json(p,
                        w);
  REQUIRE(result);
  std::string_view str(sb.GetString(),
                       sb.GetSize());
  std::string_view expected("{\"name\":\"foo\","
                             "\"value\":\"bar\"}");
  CHECK(str == expected);
}

TEST_CASE("property_parser",
          "[mcpp][yggdrasil][property][json][parse]")
{
  property p;
  property_parser handler(p);
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
                                  "\"value\":\"bar\"}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(p.name == "foo");
    CHECK(p.value == "bar");
  }
  SECTION("clear") {
    ::rapidjson::StringStream ss("{\"name\":\"foo\","
                                  "\"value\":\"bar\"}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    handler.clear();
    ::rapidjson::StringStream ss2("{\"name\":\"corge\","
                                   "\"value\":\"baz\"}");
    result = reader.Parse(ss2,
                          handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(p.name == "corge");
    CHECK(p.value == "baz");
  }
}

}
}
