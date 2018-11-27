#include <mcpp/rapidjson/array_parser.hpp>

#include <string>
#include <mcpp/rapidjson/string_parser.hpp>
#include <rapidjson/reader.h>
#include <rapidjson/stream.h>

#include <catch2/catch.hpp>

namespace mcpp::rapidjson::tests {
namespace {

TEST_CASE("array_parser",
          "[mcpp][rapidjson][json][parser]")
{
  ::rapidjson::Reader reader;
  std::vector<std::string> vec;
  array_parser<std::string,
               string_parser> parser(vec);
  SECTION("Empty array") {
    ::rapidjson::StringStream ss("[]");
    auto result = reader.Parse(ss,
                               parser);
    REQUIRE(result);
    auto ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(vec.empty());
    CHECK(parser.done());
    parser.clear();
    ::rapidjson::StringStream ss2("[]");
    result = reader.Parse(ss2,
                          parser);
    REQUIRE(result);
    ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(vec.empty());
    CHECK(parser.done());
  }
  SECTION("Non-empty array") {
    ::rapidjson::StringStream ss("[\"foo\",\"bar\",\"baz\"]");
    auto result = reader.Parse(ss,
                               parser);
    REQUIRE(result);
    auto ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(parser.done());
    auto iter = vec.begin();
    REQUIRE_FALSE(iter == vec.end());
    CHECK(*iter == "foo");
    ++iter;
    REQUIRE_FALSE(iter == vec.end());
    CHECK(*iter == "bar");
    ++iter;
    REQUIRE_FALSE(iter == vec.end());
    CHECK(*iter == "baz");
    ++iter;
    CHECK(iter == vec.end());
    parser.clear();
    ::rapidjson::StringStream ss2("[]");
    result = reader.Parse(ss2,
                          parser);
    REQUIRE(result);
    ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(vec.empty());
    CHECK(parser.done());
  }
  SECTION("Bad") {
    ::rapidjson::StringStream ss("\"foo\"");
    auto result = reader.Parse(ss,
                               parser);
    CHECK_FALSE(result);
    auto ec = parser.error_code();
    CHECK(ec);
    parser.clear();
    ::rapidjson::StringStream ss2("[]");
    result = reader.Parse(ss2,
                          parser);
    REQUIRE(result);
    ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(vec.empty());
    CHECK(parser.done());
  }
  SECTION("Nested") {
    std::vector<std::vector<std::string>> vec;
    array_parser<std::vector<std::string>,
                 array_parser<std::string,
                              string_parser>> parser(vec);
    ::rapidjson::StringStream ss("[[\"foo\"],"
                                  "[\"bar\",\"baz\"],"
                                  "[]]");
    auto result = reader.Parse(ss,
                               parser);
    REQUIRE(result);
    auto ec = parser.error_code();
    REQUIRE_FALSE(ec);
    CHECK(parser.done());
    auto iter = vec.begin();
    REQUIRE_FALSE(iter == vec.end());
    auto inner_iter = iter->begin();
    REQUIRE_FALSE(inner_iter == iter->end());
    CHECK(*inner_iter == "foo");
    ++inner_iter;
    CHECK(inner_iter == iter->end());
    ++iter;
    REQUIRE_FALSE(iter == vec.end());
    inner_iter = iter->begin();
    REQUIRE_FALSE(inner_iter == iter->end());
    CHECK(*inner_iter == "bar");
    ++inner_iter;
    REQUIRE_FALSE(inner_iter == iter->end());
    CHECK(*inner_iter == "baz");
    ++inner_iter;
    CHECK(inner_iter == iter->end());
    ++iter;
    REQUIRE_FALSE(iter == vec.end());
    CHECK(iter->empty());
    ++iter;
    CHECK(iter == vec.end());
  }
}

}
}
