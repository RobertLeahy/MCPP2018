#include <mcpp/rapidjson/string_parser.hpp>

#include <string>
#include <rapidjson/reader.h>
#include <rapidjson/stream.h>

#include <catch2/catch.hpp>

namespace mcpp::rapidjson::tests {
namespace {

TEST_CASE("string_parser",
          "[mcpp][rapidjson][json][parser]")
{
  ::rapidjson::Reader reader;
  std::string str;
  string_parser parser(str);
  SECTION("Good") {
    ::rapidjson::StringStream ss("\"foo\"");
    auto result = reader.Parse(ss,
                               parser);
    REQUIRE(result);
    auto ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(str == "foo");
    CHECK(parser.done());
    parser.clear();
    REQUIRE_FALSE(parser.done());
    ::rapidjson::StringStream ss2("\"bar\"");
    result = reader.Parse(ss2,
                          parser);
    REQUIRE(result);
    ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(str == "bar");
    CHECK(parser.done());
  }
  SECTION("Bad") {
    ::rapidjson::StringStream ss("{}");
    auto result = reader.Parse(ss,
                               parser);
    CHECK_FALSE(result);
    auto ec = parser.error_code();
    CHECK(ec);
    parser.clear();
    ::rapidjson::StringStream ss2("\"bar\"");
    result = reader.Parse(ss2,
                          parser);
    REQUIRE(result);
    ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(str == "bar");
    CHECK(parser.done());
  }
}

}
}
