#include <mcpp/yggdrasil/profile.hpp>

#include <string>
#include <string_view>
#include <rapidjson/reader.h>
#include <rapidjson/stream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <catch2/catch.hpp>

namespace mcpp::yggdrasil::tests {
namespace {

TEST_CASE("to_json (profile)",
          "[mcpp][yggdrasil][profile][to_json][json]")
{
  ::rapidjson::StringBuffer sb;
  ::rapidjson::Writer<::rapidjson::StringBuffer> w(sb);
  profile p;
  p.id = "foo";
  p.name = "bar";
  SECTION("legacy = false") {
    auto result = to_json(p,
                          w);
    REQUIRE(result);
    std::string_view str(sb.GetString(),
                         sb.GetSize());
    std::string_view expected("{\"id\":\"foo\","
                               "\"name\":\"bar\"}");
    CHECK(str == expected);
  }
  SECTION("legacy = true") {
    p.legacy = true;
    auto result = to_json(p,
                          w);
    REQUIRE(result);
    std::string_view str(sb.GetString(),
                         sb.GetSize());
    std::string_view expected("{\"id\":\"foo\","
                               "\"name\":\"bar\","
                               "\"legacy\":true}");
    CHECK(str == expected);
  }
}

TEST_CASE("profile_parser",
          "[mcpp][yggdrasil][profile][json][parse]")
{
  profile p;
  profile_parser handler(p);
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
  SECTION("Good (legacy = false)") {
    ::rapidjson::StringStream ss("{\"id\":\"foo\","
                                  "\"name\":\"bar\","
                                  "\"legacy\":false}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(p.id == "foo");
    CHECK(p.name == "bar");
    CHECK_FALSE(p.legacy);
  }
  SECTION("Good (legacy = true)") {
    ::rapidjson::StringStream ss("{\"id\":\"foo\","
                                  "\"name\":\"bar\","
                                  "\"legacy\":true}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(p.id == "foo");
    CHECK(p.name == "bar");
    CHECK(p.legacy);
  }
  SECTION("Good (no legacy)") {
    ::rapidjson::StringStream ss("{\"id\":\"foo\","
                                  "\"name\":\"bar\"}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(p.id == "foo");
    CHECK(p.name == "bar");
    CHECK_FALSE(p.legacy);
  }
  SECTION("clear") {
    ::rapidjson::StringStream ss("{\"id\":\"foo\","
                                  "\"name\":\"bar\","
                                  "\"legacy\":true}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(p.legacy);
    handler.clear();
    ::rapidjson::StringStream ss2("{\"id\":\"foo\","
                                   "\"name\":\"bar\"}");
    result = reader.Parse(ss2,
                          handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(p.id == "foo");
    CHECK(p.name == "bar");
    CHECK_FALSE(p.legacy);
  }
}

}
}
