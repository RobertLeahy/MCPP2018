#include <mcpp/system_error.hpp>

#include <string>
#include <system_error>
#include <boost/asio/error.hpp>
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

TEST_CASE("to_boost_error_code",
          "[mcpp][system_error][core]")
{
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Tests/To Boost Error Code";
    }
    virtual std::string message(int code) const noexcept override {
      return "This is a test";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      if (!code) {
        return std::error_condition();
      }
      return make_error_code(std::errc::invalid_argument).default_error_condition();
    }
  } category;
  std::error_code sec(1,
                      category);
  CHECK(sec);
  auto ec = to_boost_error_code(sec);
  CHECK(ec);
  CHECK(ec.category().name() == std::string("MCPP/Tests/To Boost Error Code"));
  CHECK(ec.message() == "This is a test");
  CHECK(ec.default_error_condition() == to_boost_error_code(make_error_code(std::errc::invalid_argument)).default_error_condition());
  sec = make_error_code(boost::asio::error::eof);
  ec = to_boost_error_code(sec);
  CHECK(is_eof(ec));
}

}
}
