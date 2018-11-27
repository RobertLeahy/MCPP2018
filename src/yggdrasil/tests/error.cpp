#include <mcpp/yggdrasil/error.hpp>

#include <string>
#include <string_view>
#include <rapidjson/reader.h>
#include <rapidjson/stream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <catch2/catch.hpp>

namespace mcpp::yggdrasil::tests {
namespace {

TEST_CASE("to_json (error)",
          "[mcpp][yggdrasil][error][to_json][json]")
{
  ::rapidjson::StringBuffer sb;
  ::rapidjson::Writer<::rapidjson::StringBuffer> w(sb);
  error e;
  e.error = "test error";
  e.error_message = "test error message";
  SECTION("No cause") {
    auto result = to_json(e,
                          w);
    REQUIRE(result);
    std::string_view str(sb.GetString(),
                         sb.GetSize());
    std::string_view expected("{\"error\":\"test error\","
                               "\"errorMessage\":\"test error message\"}");
    CHECK(str == expected);
  }
  SECTION("Cause") {
    e.cause = "test cause";
    auto result = to_json(e,
                          w);
    REQUIRE(result);
    std::string_view str(sb.GetString(),
                         sb.GetSize());
    std::string_view expected("{\"error\":\"test error\","
                               "\"errorMessage\":\"test error message\","
                               "\"cause\":\"test cause\"}");
    CHECK(str == expected);
  }
}

TEST_CASE("error_parser",
          "[mcpp][yggdrasil][error][json][parse]")
{
  error obj;
  error_parser handler(obj);
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
    ::rapidjson::StringStream ss("\"foo\"");
    auto result = reader.Parse(ss,
                               handler);
    CHECK_FALSE(result);
    auto ec = handler.error_code();
    CHECK(ec);
  }
  SECTION("Good (no cause)") {
    ::rapidjson::StringStream ss("{\"error\":\"test error\","
                                  "\"errorMessage\":\"test error message\"}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(obj.error == "test error");
    CHECK(obj.error_message == "test error message");
    CHECK_FALSE(obj.cause);
  }
  SECTION("Good (cause)") {
    ::rapidjson::StringStream ss("{\"error\":\"test error\","
                                  "\"errorMessage\":\"test error message\","
                                  "\"cause\":\"test cause\"}");
    auto result = reader.Parse(ss,
                               handler);
    REQUIRE(result);
    REQUIRE_FALSE(handler.error_code());
    CHECK(obj.error == "test error");
    CHECK(obj.error_message == "test error message");
    REQUIRE(obj.cause);
    CHECK(*obj.cause == "test cause");
  }
}

}
}
