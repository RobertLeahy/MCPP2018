#include <mcpp/yggdrasil/user.hpp>

#include <string>
#include <string_view>
#include <mcpp/yggdrasil/property.hpp>
#include <rapidjson/reader.h>
#include <rapidjson/stream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <catch2/catch.hpp>

namespace mcpp::yggdrasil::tests {
namespace {

TEST_CASE("to_json (user)",
          "[mcpp][yggdrasil][user][to_json][json]")
{
  ::rapidjson::StringBuffer sb;
  ::rapidjson::Writer<::rapidjson::StringBuffer> w(sb);
  user u;
  u.id = "foo";
  SECTION("No properties") {
    auto result = to_json(u,
                          w);
    REQUIRE(result);
    std::string_view str(sb.GetString(),
                         sb.GetSize());
    std::string_view expected("{\"id\":\"foo\","
                               "\"properties\":[]}");
    CHECK(str == expected);
  }
  SECTION("Properties") {
    property p;
    p.name = "bar";
    p.value = "corge";
    u.properties.push_back(p);
    p.name = "quux";
    p.value = "baz";
    u.properties.push_back(p);
    auto result = to_json(u,
                          w);
    REQUIRE(result);
    std::string_view str(sb.GetString(),
                         sb.GetSize());
    std::string_view expected("{\"id\":\"foo\","
                               "\"properties\":[{\"name\":\"bar\","
                                                "\"value\":\"corge\"},"
                                               "{\"name\":\"quux\","
                                                "\"value\":\"baz\"}]}");
    CHECK(str == expected);
  }
}

TEST_CASE("user_parser",
          "[mcpp][yggdrasil][user][json][parse]")
{
  user u;
  user_parser handler(u);
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
  SECTION("Good (no properties)") {
    ::rapidjson::StringStream ss("{\"id\":\"foo\","
                                  "\"properties\":[{\"name\":\"bar\","
                                                   "\"value\":\"corge\"},"
                                                  "{\"name\":\"quux\","
                                                   "\"value\":\"baz\"}]}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(u.id == "foo");
    auto iter = u.properties.begin();
    REQUIRE_FALSE(iter == u.properties.end());
    ++iter;
    REQUIRE_FALSE(iter == u.properties.end());
    ++iter;
    CHECK(iter == u.properties.end());
  }
  SECTION("Good (no properties)") {
    ::rapidjson::StringStream ss("{\"id\":\"foo\","
                                  "\"properties\":[]}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(u.id == "foo");
    CHECK(u.properties.empty());
  }
  SECTION("clear") {
    ::rapidjson::StringStream ss("{\"id\":\"foo\","
                                  "\"properties\":[{\"name\":\"bar\","
                                                   "\"value\":\"corge\"},"
                                                  "{\"name\":\"quux\","
                                                   "\"value\":\"baz\"}]}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK_FALSE(u.properties.empty());
    handler.clear();
    ::rapidjson::StringStream ss2("{\"id\":\"foo\","
                                   "\"properties\":[]}");
    result = reader.Parse(ss2,
                          handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(u.id == "foo");
    CHECK(u.properties.empty());
  }
}

}
}
