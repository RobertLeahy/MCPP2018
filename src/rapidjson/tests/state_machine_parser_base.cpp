#include <mcpp/rapidjson/state_machine_parser_base.hpp>

#include <string>
#include <mcpp/rapidjson/string_parser.hpp>
#include <rapidjson/reader.h>
#include <rapidjson/stream.h>

#include <catch2/catch.hpp>

namespace mcpp::rapidjson::tests {
namespace {

TEST_CASE("state_machine_parser_base (no template parameters)",
          "[mcpp][rapidjson][state_machine_parser_base][parser][json]")
{
  ::rapidjson::Reader reader;
  ::rapidjson::StringStream ss("{}");
  state_machine_parser_base<> parser;
  auto result = reader.Parse(ss,
                             parser);
  CHECK_FALSE(result);
  auto ec = parser.error_code();
  CHECK(ec);
}

class mock_state_machine_parser : public state_machine_parser_base<string_parser> {
public:
  using state_machine_parser_base<string_parser>::emplace;
};

TEST_CASE("state_machine_parser_base (template parameters)",
          "[mcpp][rapidjson][state_machine_parser_base][parser][json]")
{
  ::rapidjson::Reader reader;
  ::rapidjson::StringStream ss("\"foo\"");
  std::string str;
  mock_state_machine_parser parser;
  SECTION("No parser") {
    auto result = reader.Parse(ss,
                               parser);
    CHECK_FALSE(result);
    auto ec = parser.error_code();
    CHECK(ec);
  }
  SECTION("Parser") {
    parser.emplace<string_parser>(str);
    auto result = reader.Parse(ss,
                               parser);
    REQUIRE(result);
    auto ec = parser.error_code();
    REQUIRE_FALSE(ec);
    REQUIRE(parser.done());
    CHECK(str == "foo");
  }
}

}
}
