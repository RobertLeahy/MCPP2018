#include <mcpp/try_copy_n.hpp>

#include <iterator>
#include <list>
#include <system_error>
#include <vector>
#include <boost/asio/error.hpp>
#include <mcpp/system_error.hpp>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

TEST_CASE("try_copy_n w/RandomAccessIterator",
          "[mcpp][try_copy_n][algorithm][core]")
{
  std::vector<int> in;
  std::vector<int> out;
  std::error_code ec;
  SECTION("Fail") {
    auto pair = try_copy_n(in.begin(),
                           in.end(),
                           1,
                           std::back_inserter(out),
                           ec);
    CHECK(ec);
    CHECK(is_eof(ec));
    CHECK(pair.first == in.end());
  }
  SECTION("Success") {
    in.push_back(1);
    in.push_back(2);
    in.push_back(3);
    auto pair = try_copy_n(in.begin(),
                           in.end(),
                           2,
                           std::back_inserter(out),
                           ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.first == (in.end() - 1));
    REQUIRE(out.size() == 2);
    CHECK(out[0] == 1);
    CHECK(out[1] == 2);
  }
  SECTION("Negative") {
    auto pair = try_copy_n(in.begin(),
                           in.end(),
                           -1,
                           std::back_inserter(out),
                           ec);
    CHECK(ec);
    //  TODO
    CHECK(pair.first == in.begin());
  }
}

TEST_CASE("try_copy_n w/InputIterator",
          "[mcpp][try_copy_n][algorithm][core]")
{
  std::list<int> in;
  std::vector<int> out;
  std::error_code ec;
  SECTION("Fail") {
    auto pair = try_copy_n(in.begin(),
                           in.end(),
                           1,
                           std::back_inserter(out),
                           ec);
    CHECK(ec);
    CHECK(is_eof(ec));
    CHECK(pair.first == in.end());
  }
  SECTION("Success") {
    in.push_back(1);
    in.push_back(2);
    in.push_back(3);
    auto pair = try_copy_n(in.begin(),
                           in.end(),
                           2,
                           std::back_inserter(out),
                           ec);
    REQUIRE_FALSE(ec);
    auto expected = in.end();
    --expected;
    CHECK(pair.first == expected);
    REQUIRE(out.size() == 2);
    CHECK(out[0] == 1);
    CHECK(out[1] == 2);
  }
  SECTION("Negative") {
    auto pair = try_copy_n(in.begin(),
                           in.end(),
                           -1,
                           std::back_inserter(out),
                           ec);
    CHECK(ec);
    CHECK(make_error_code(std::errc::argument_out_of_domain).default_error_condition() == ec.default_error_condition());
    CHECK(pair.first == in.begin());
  }
}

}
}
