#include <mcpp/serialization/endian.hpp>

#include <cmath>
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

TEST_CASE("to_endian",
          "[mcpp][endian][serialization")
{
  std::vector<std::byte> buffer;
  SECTION("Integer") {
    to_endian(std::uint16_t(1),
              std::back_inserter(buffer));
    REQUIRE(buffer.size() == 2);
    CHECK(buffer[0] == std::byte{0});
    CHECK(buffer[1] == std::byte{1});
  }
  SECTION("Floating point") {
    static_assert(sizeof(float) == 4);
    to_endian(std::numeric_limits<float>::infinity(),
              std::back_inserter(buffer));
    REQUIRE(buffer.size() == 4);
    CHECK(buffer[0] == std::byte{0b01111111});
    CHECK(buffer[1] == std::byte{0b10000000});
    CHECK(buffer[2] == std::byte{0});
    CHECK(buffer[3] == std::byte{0});
  }
}

TEST_CASE("from_endian",
          "[mcpp][endian][serialization")
{
  std::vector<std::byte> buffer;
  std::error_code ec;
  SECTION("Integer") {
    buffer.push_back(std::byte{0});
    buffer.push_back(std::byte{1});
    auto pair = from_endian<std::uint16_t>(buffer.begin(),
                                           buffer.end(),
                                           ec);
    CHECK(pair.second == buffer.end());
    REQUIRE_FALSE(ec);
    CHECK(pair.first == 1);
  }
  SECTION("Too much data") {
    buffer.push_back(std::byte{0});
    buffer.push_back(std::byte{1});
    buffer.push_back(std::byte{2});
    auto pair = from_endian<std::uint16_t>(buffer.begin(),
                                           buffer.end(),
                                           ec);
    CHECK(pair.second == (buffer.end() - 1));
    REQUIRE_FALSE(ec);
    CHECK(pair.first == 1);
  }
  SECTION("End of file") {
    buffer.push_back(std::byte{0});
    auto pair = from_endian<std::uint16_t>(buffer.begin(),
                                           buffer.end(),
                                           ec);
    CHECK(pair.second == buffer.end());
    CHECK(ec);
    CHECK(ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition());
  }
  SECTION("Floating point") {
    static_assert(sizeof(float) == 4);
    buffer.push_back(std::byte{0b01111111});
    buffer.push_back(std::byte{0b10000000});
    buffer.push_back(std::byte{0});
    buffer.push_back(std::byte{0});
    auto pair = from_endian<float>(buffer.begin(),
                                   buffer.end(),
                                   ec);
    CHECK(pair.second == buffer.end());
    REQUIRE_FALSE(ec);
    CHECK(std::isinf(pair.first));
    CHECK_FALSE(std::signbit(pair.first));
  }
}

}
}
