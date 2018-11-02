#include <mcpp/serialization/string.hpp>

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <list>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>
#include <boost/asio/error.hpp>
#include <mcpp/system_error.hpp>

#include <catch2/catch.hpp>

namespace mcpp::serialization {
namespace {

TEST_CASE("string_check",
          "[mcpp][string][serialization]")
{
  CHECK(string_check(std::string()));
  CHECK(string_check(std::string("foo")));
  CHECK(string_check("bar"));
  std::string str;
  std::string::size_type size = std::numeric_limits<std::int16_t>::max();
  ++size;
  str.resize(size,
             'a');
  CHECK_FALSE(string_check(str));
}

TEST_CASE("string_size",
          "[mcpp][string][serialization]")
{
  CHECK(string_size(std::string()) == 1);
  CHECK(string_size(std::string("foo")) == 4);
  CHECK(string_size("bar") == 4);
  std::string str;
  std::string::size_type size = std::numeric_limits<std::int16_t>::max();
  ++size;
  str.resize(size,
             'a');
  CHECK_THROWS_AS(string_size(str),
                  std::system_error);
}

TEST_CASE("to_string",
          "[mcpp][string][serialization]")
{
  std::vector<std::byte> buffer;
  SECTION("Empty") {
    to_string("",
              std::back_inserter(buffer));
    REQUIRE(buffer.size() == 1);
    CHECK(buffer.front() == std::byte{0});
  }
  SECTION("Non-empty") {
    to_string("foo",
              std::back_inserter(buffer));
    REQUIRE(buffer.size() == 4);
    CHECK(buffer[0] == std::byte{3});
    CHECK(buffer[1] == std::byte{'f'});
    CHECK(buffer[2] == std::byte{'o'});
    CHECK(buffer[3] == std::byte{'o'});
  }
  SECTION("Contains U+0000") {
    std::string str;
    str.push_back('\0');
    to_string(str,
              std::back_inserter(buffer));
    REQUIRE(buffer.size() == 2);
    CHECK(buffer[0] == std::byte{1});
    CHECK(buffer[1] == std::byte{0});
  }
  SECTION("Too long") {
    std::string str;
    std::string::size_type size = std::numeric_limits<std::int16_t>::max();
    ++size;
    str.resize(size,
               'a');
    CHECK_THROWS_AS(to_string(str,
                              std::back_inserter(buffer)),
                    std::system_error);
  }
}

TEST_CASE("from_string",
          "[mcpp][string][serialization]")
{
  std::vector<std::byte> buffer;
  std::error_code ec;
  SECTION("Empty") {
    buffer.push_back(std::byte{0});
    auto pair = from_string<std::string>(buffer.begin(),
                                         buffer.end(),
                                         ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == buffer.end());
    CHECK(pair.first.empty());
  }
  SECTION("Non-empty") {
    buffer.push_back(std::byte{3});
    buffer.push_back(std::byte{'f'});
    buffer.push_back(std::byte{'o'});
    buffer.push_back(std::byte{'o'});
    auto pair = from_string<std::string>(buffer.begin(),
                                         buffer.end(),
                                         ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == buffer.end());
    CHECK(pair.first == "foo");
  }
  SECTION("Incomplete") {
    buffer.push_back(std::byte{3});
    buffer.push_back(std::byte{'f'});
    buffer.push_back(std::byte{'o'});
    auto pair = from_string<std::string>(buffer.begin(),
                                         buffer.end(),
                                         ec);
    CHECK(ec);
    CHECK(is_eof(ec));
    CHECK(pair.second == buffer.end());
    CHECK(pair.first == "");
  }
  SECTION("Non-random access iterator") {
    std::list<std::byte> l;
    l.push_back(std::byte{3});
    l.push_back(std::byte{'f'});
    l.push_back(std::byte{'o'});
    l.push_back(std::byte{'o'});
    auto pair = from_string<std::string>(l.begin(),
                                         l.end(),
                                         ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == l.end());
    CHECK(pair.first == "foo");
  }
  SECTION("Non-random access iterator incomplete") {
    std::list<std::byte> l;
    l.push_back(std::byte{3});
    l.push_back(std::byte{'f'});
    l.push_back(std::byte{'o'});
    auto pair = from_string<std::string>(l.begin(),
                                         l.end(),
                                         ec);
    CHECK(ec);
    CHECK(is_eof(ec));
    CHECK(pair.second == l.end());
    CHECK(pair.first == "");
  }
}

}
}
