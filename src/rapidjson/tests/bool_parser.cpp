#include <mcpp/rapidjson/bool_parser.hpp>

#include <rapidjson/reader.h>
#include <rapidjson/stream.h>

#include <catch2/catch.hpp>

namespace mcpp::rapidjson::tests {
namespace {

TEST_CASE("bool_parser",
          "[mcpp][rapidjson][json][parser]")
{
  ::rapidjson::Reader reader;
  bool b = false;
  bool_parser parser(b);
  SECTION("Good") {
    ::rapidjson::StringStream ss("true");
    auto result = reader.Parse(ss,
                               parser);
    REQUIRE(result);
    auto ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(b);
    CHECK(parser.done());
    parser.clear();
    REQUIRE_FALSE(parser.done());
    ::rapidjson::StringStream ss2("false");
    result = reader.Parse(ss2,
                          parser);
    REQUIRE(result);
    ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK_FALSE(b);
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
    ::rapidjson::StringStream ss2("true");
    result = reader.Parse(ss2,
                          parser);
    REQUIRE(result);
    ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(b);
    CHECK(parser.done());
  }
}

}
}
