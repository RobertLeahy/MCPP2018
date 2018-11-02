#include <mcpp/serialization/varint.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <system_error>
#include <vector>
#include <boost/asio/error.hpp>
#include <mcpp/system_error.hpp>

#include <catch2/catch.hpp>

namespace mcpp::serialization::tests {
namespace {

static_assert(varint_max_size<std::int64_t> == 10);
static_assert(varint_max_size<std::int32_t> == 5);
static_assert(varint_max_size<std::int16_t> == 3);
static_assert(varint_max_size<std::uint64_t> == 10);
static_assert(varint_max_size<std::uint32_t> == 5);
static_assert(varint_max_size<std::uint16_t> == 3);
static_assert(varint_size(5) == 1);
static_assert(varint_size(128) == 2);
static_assert(varint_size(std::int64_t(-1)) == 10);
static_assert(varint_size(std::int32_t(-1)) == 5);
static_assert(varint_size(std::int16_t(-1)) == 3);

TEST_CASE("to_varint",
          "[mcpp][varint][serialization]")
{
  std::array<std::byte,
             varint_max_size<std::int32_t>> buffer;
  SECTION("Zero") {
    auto iter = to_varint(std::int32_t(0),
                          buffer.begin());
    REQUIRE(std::distance(buffer.begin(),
                          iter) == 1);
    CHECK(buffer.front() == std::byte{0});
  }
  SECTION("Single byte") {
    auto iter = to_varint(std::int32_t(1),
                          buffer.begin());
    REQUIRE(std::distance(buffer.begin(),
                          iter) == 1);
    CHECK(buffer.front() == std::byte{1});
  }
  SECTION("Negative") {
    auto iter = to_varint(std::int32_t(-1),
                          buffer.begin());
    REQUIRE(iter == buffer.end());
    CHECK(buffer[0] == std::byte{0b11111111});
    CHECK(buffer[1] == std::byte{0b11111111});
    CHECK(buffer[2] == std::byte{0b11111111});
    CHECK(buffer[3] == std::byte{0b11111111});
    CHECK(buffer[4] == std::byte{0b00001111});
  }
  SECTION("300") {
    auto iter = to_varint(std::int32_t(300),
                          buffer.begin());
    REQUIRE(std::distance(buffer.begin(),
                          iter) == 2);
    CHECK(buffer[0] == std::byte{0b10101100});
    CHECK(buffer[1] == std::byte{0b00000010});
  }
}

TEST_CASE("to_zig_zag_varint",
          "[mcpp][varint][serialization]")
{
  std::array<std::byte,
             varint_max_size<std::int32_t>> buffer;
  SECTION("Zero") {
    auto iter = to_zig_zag_varint(std::int32_t(0),
                                  buffer.begin());
    REQUIRE(std::distance(buffer.begin(),
                          iter) == 1);
    CHECK(buffer.front() == std::byte{0});
  }
  SECTION("Positive") {
    auto iter = to_zig_zag_varint(std::int32_t(1),
                                  buffer.begin());
    REQUIRE(std::distance(buffer.begin(),
                          iter) == 1);
    CHECK(buffer.front() == std::byte{2});
  }
  SECTION("Negative") {
    auto iter = to_zig_zag_varint(std::int32_t(-1),
                                  buffer.begin());
    REQUIRE(std::distance(buffer.begin(),
                          iter) == 1);
    CHECK(buffer.front() == std::byte{1});
  }
  SECTION("Max") {
    auto iter = to_zig_zag_varint(std::numeric_limits<std::int32_t>::max(),
                                  buffer.begin());
    REQUIRE(iter == buffer.end());
    CHECK(buffer[0] == std::byte{0b11111110});
    CHECK(buffer[1] == std::byte{0b11111111});
    CHECK(buffer[2] == std::byte{0b11111111});
    CHECK(buffer[3] == std::byte{0b11111111});
    CHECK(buffer[4] == std::byte{0b00001111});
  }
  SECTION("Min") {
    auto iter = to_zig_zag_varint(std::numeric_limits<std::int32_t>::min(),
                                  buffer.begin());
    REQUIRE(iter == buffer.end());
    CHECK(buffer[0] == std::byte{0b11111111});
    CHECK(buffer[1] == std::byte{0b11111111});
    CHECK(buffer[2] == std::byte{0b11111111});
    CHECK(buffer[3] == std::byte{0b11111111});
    CHECK(buffer[4] == std::byte{0b00001111});
  }
}

TEST_CASE("from_varint",
          "[mcpp][varint][serialization]")
{
  std::vector<std::byte> vec;
  std::error_code ec;
  SECTION("Empty") {
    auto pair = from_varint<int>(vec.begin(),
                                 vec.end(),
                                 ec);
    CHECK(ec);
    CHECK(is_eof(ec));
    CHECK(pair.second == vec.end());
  }
  SECTION("End of file") {
    vec.push_back(std::byte{0b11111111});
    auto pair = from_varint<int>(vec.begin(),
                                 vec.end(),
                                 ec);
    CHECK(ec);
    CHECK(is_eof(ec));
    CHECK(pair.second == vec.end());
  }
  SECTION("Single byte") {
    vec.push_back(std::byte{1});
    auto pair = from_varint<int>(vec.begin(),
                                 vec.end(),
                                 ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == vec.end());
    CHECK(pair.first == 1);
  }
  SECTION("300") {
    vec.push_back(std::byte{0b10101100});
    vec.push_back(std::byte{0b00000010});
    auto pair = from_varint<int>(vec.begin(),
                                 vec.end(),
                                 ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == vec.end());
    CHECK(pair.first == 300);
  }
  SECTION("Negative") {
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b00001111});
    auto pair = from_varint<std::int32_t>(vec.begin(),
                                          vec.end(),
                                          ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == vec.end());
    CHECK(pair.first == -1);
  }
  SECTION("Overflow") {
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b00011111});
    auto pair = from_varint<std::int32_t>(vec.begin(),
                                          vec.end(),
                                          ec);
    CHECK(ec);
    CHECK(make_error_code(std::errc::value_too_large).default_error_condition() == ec.default_error_condition());
    CHECK(pair.second == (vec.end() - 1));
  }
  SECTION("Longer than max") {
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b10001111});
    vec.push_back(std::byte{0});
    auto pair = from_varint<std::int32_t>(vec.begin(),
                                          vec.end(),
                                          ec);
    CHECK(ec);
    CHECK(make_error_code(std::errc::value_too_large).default_error_condition() == ec.default_error_condition());
    CHECK(pair.second == (vec.end() - 1));
  }
  SECTION("Extra bytes") {
    vec.push_back(std::byte{0b10101100});
    vec.push_back(std::byte{0b00000010});
    vec.push_back(std::byte{0});
    auto pair = from_varint<int>(vec.begin(),
                                 vec.end(),
                                 ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == (vec.end() - 1));
    CHECK(pair.first == 300);
  }
  SECTION("Overlong") {
    vec.push_back(std::byte{0b10101100});
    vec.push_back(std::byte{0b10000010});
    vec.push_back(std::byte{0});
    auto pair = from_varint<int>(vec.begin(),
                                 vec.end(),
                                 ec);
    CHECK(ec);
    CHECK(make_error_code(std::errc::bad_message).default_error_condition() == ec.default_error_condition());
    CHECK(pair.second == (vec.end() - 1));
  }
  SECTION("Zero") {
    vec.push_back(std::byte{0});
    auto pair = from_varint<int>(vec.begin(),
                                 vec.end(),
                                 ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == vec.end());
    CHECK(pair.first == 0);
  }
}

TEST_CASE("from_zig_zag_varint",
          "[mcpp][varint][serialization]")
{
  std::vector<std::byte> vec;
  std::error_code ec;
  SECTION("Zero") {
    vec.push_back(std::byte{0});
    auto pair = from_zig_zag_varint<int>(vec.begin(),
                                         vec.end(),
                                         ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == vec.end());
    CHECK(pair.first == 0);
  }
  SECTION("Positive") {
    vec.push_back(std::byte{1});
    auto pair = from_zig_zag_varint<int>(vec.begin(),
                                         vec.end(),
                                         ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == vec.end());
    CHECK(pair.first == -1);
  }
  SECTION("Negative") {
    vec.push_back(std::byte{2});
    auto pair = from_zig_zag_varint<int>(vec.begin(),
                                         vec.end(),
                                         ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == vec.end());
    CHECK(pair.first == 1);
  }
  SECTION("Min") {
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b00001111});
    auto pair = from_zig_zag_varint<std::int32_t>(vec.begin(),
                                                  vec.end(),
                                                  ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == vec.end());
    CHECK(pair.first == std::numeric_limits<std::int32_t>::min());
  }
  SECTION("Min") {
    vec.push_back(std::byte{0b11111110});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b11111111});
    vec.push_back(std::byte{0b00001111});
    auto pair = from_zig_zag_varint<std::int32_t>(vec.begin(),
                                                  vec.end(),
                                                  ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.second == vec.end());
    CHECK(pair.first == std::numeric_limits<std::int32_t>::max());
  }
}

}
}
