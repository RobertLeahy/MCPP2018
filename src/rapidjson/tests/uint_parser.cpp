#include <mcpp/rapidjson/uint_parser.hpp>

#include <rapidjson/reader.h>
#include <rapidjson/stream.h>

#include <catch2/catch.hpp>

namespace mcpp::rapidjson::tests {
namespace {

TEST_CASE("uint_parser",
          "[mcpp][rapidjson][json][parser]")
{
  ::rapidjson::Reader reader;
  unsigned u;
  uint_parser parser(u);
  SECTION("Good") {
    ::rapidjson::StringStream ss("5");
    auto result = reader.Parse(ss,
                               parser);
    REQUIRE(result);
    auto ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(u == 5);
    CHECK(parser.done());
    parser.clear();
    REQUIRE_FALSE(parser.done());
    ::rapidjson::StringStream ss2("6");
    result = reader.Parse(ss2,
                          parser);
    REQUIRE(result);
    ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(u == 6);
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
    ::rapidjson::StringStream ss2("6");
    result = reader.Parse(ss2,
                          parser);
    REQUIRE(result);
    ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(u == 6);
    CHECK(parser.done());
  }
}

}
}
