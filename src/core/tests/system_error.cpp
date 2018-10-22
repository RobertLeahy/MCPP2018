#include <mcpp/system_error.hpp>

#include <system_error>
#include <boost/system/error_code.hpp>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

TEST_CASE("to_error_code",
          "[mcpp][system_error][core]")
{
  SECTION("No error") {
    boost::system::error_code ec;
    auto sec = to_error_code(ec);
    CHECK_FALSE(sec);
    CHECK(sec.value() == ec.value());
    CHECK(sec.message() == ec.message());
  }
  SECTION("Error") {
    boost::system::error_code ec = make_error_code(boost::system::errc::not_enough_memory);
    auto sec = to_error_code(ec);
    CHECK(sec);
    CHECK(sec.value() == ec.value());
    CHECK(sec.message() == ec.message());
  }
}

}
}
