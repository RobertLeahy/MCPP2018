#include <mcpp/checked.hpp>

#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

TEST_CASE("checked_cast (integer)") {
  SECTION("unsigned => signed (same width)") {
    std::uint16_t u = std::numeric_limits<std::uint16_t>::max();
    auto result = checked_cast<std::int16_t>(u);
    CHECK_FALSE(result);
    u = std::uint16_t(std::numeric_limits<std::int16_t>::max());
    result = checked_cast<std::int16_t>(u);
    REQUIRE(result);
    CHECK(*result == std::numeric_limits<std::int16_t>::max());
  }
  SECTION("signed => unsigned (same width)") {
    std::int16_t i = std::numeric_limits<std::int16_t>::max();
    auto result = checked_cast<std::uint16_t>(i);
    REQUIRE(result);
    CHECK(*result == std::uint16_t(std::numeric_limits<std::int16_t>::max()));
    i = -1;
    result = checked_cast<std::uint16_t>(i);
    CHECK_FALSE(result);
  }
  SECTION("signed => signed (wider)") {
    std::int16_t i = std::numeric_limits<std::int16_t>::max();
    auto result = checked_cast<std::int32_t>(i);
    REQUIRE(result);
    CHECK(*result == i);
  }
  SECTION("signed => signed (narrower)") {
    std::int32_t i = std::numeric_limits<std::int32_t>::max();
    auto result = checked_cast<std::int16_t>(i);
    CHECK_FALSE(result);
    i = std::int32_t(std::numeric_limits<std::int16_t>::max());
    result = checked_cast<std::int16_t>(i);
    REQUIRE(result);
    CHECK(result == std::numeric_limits<std::int16_t>::max());
    i = std::numeric_limits<std::int32_t>::min();
    result = checked_cast<std::int16_t>(i);
    CHECK_FALSE(result);
    i = std::int32_t(std::numeric_limits<std::int16_t>::min());
    result = checked_cast<std::int16_t>(i);
    REQUIRE(result);
    CHECK(result == std::numeric_limits<std::int16_t>::min());
  }
  SECTION("unsigned => unsigned (wider)") {
    std::uint16_t u = std::numeric_limits<std::uint16_t>::max();
    auto result = checked_cast<std::uint32_t>(u);
    REQUIRE(result);
    CHECK(*result == u);
  }
  SECTION("unsigned => unsigned (narrower)") {
    std::uint32_t u = std::numeric_limits<std::uint32_t>::max();
    auto result = checked_cast<std::uint16_t>(u);
    CHECK_FALSE(result);
    u = std::numeric_limits<std::uint16_t>::max();
    result = checked_cast<std::uint16_t>(u);
    REQUIRE(result);
    CHECK(*result == std::numeric_limits<std::uint16_t>::max());
  }
}

TEST_CASE("checked_cast (optional)") {
  SECTION("Empty") {
    std::optional<int> in;
    auto out = checked_cast<short>(in);
    CHECK_FALSE(out);
  }
  SECTION("Not empty, success") {
    std::optional<int> in(5);
    auto out = checked_cast<short>(in);
    REQUIRE(out);
    CHECK(*out == 5);
  }
  SECTION("Not empty, failure") {
    std::optional<int> in(-1);
    auto out = checked_cast<unsigned>(in);
    CHECK_FALSE(out);
  }
}

TEST_CASE("checked_add (integers)") {
  SECTION("No overflow, 2 operands") {
    unsigned u(5);
    int i(4);
    auto result = checked_add(u,
                              i);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned>>);
    REQUIRE(result);
    CHECK(*result == 9);
  }
  SECTION("No overflow, 3 operands") {
    unsigned u(5);
    int i(4);
    unsigned long ul(16);
    auto result = checked_add(u,
                              i,
                              ul);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned long>>);
    REQUIRE(result);
    CHECK(*result == 25);
  }
  SECTION("No overflow, 4 operands") {
    unsigned u(5);
    int i(4);
    unsigned long ul(16);
    long l(18);
    auto result = checked_add(u,
                              i,
                              ul,
                              l);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned long>>);
    REQUIRE(result);
    CHECK(*result == 43);
  }
  SECTION("Overflow, unsigned") {
    unsigned a = 1;
    unsigned b = std::numeric_limits<unsigned>::max();
    auto result = checked_add(a,
                              b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned>>);
    CHECK_FALSE(result);
    result = checked_add(b,
                         a);
    CHECK_FALSE(result);
  }
  SECTION("Overflow, signed") {
    int a = 1;
    int b = std::numeric_limits<int>::max();
    auto result = checked_add(a,
                              b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<int>>);
    CHECK_FALSE(result);
    result = checked_add(b,
                         a);
    CHECK_FALSE(result);
    a = -1;
    b = std::numeric_limits<int>::min();
    result = checked_add(a,
                         b);
    CHECK_FALSE(result);
    result = checked_add(b,
                         a);
    CHECK_FALSE(result);
  }
  SECTION("Overflow, mixed") {
    unsigned a = 1;
    int b = -2;
    auto result = checked_add(a,
                              b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned>>);
    CHECK_FALSE(result);
    result = checked_add(b,
                         a);
    CHECK_FALSE(result);
  }
}

TEST_CASE("checked_add (optional)") {
  SECTION("Both optional, empty") {
    std::optional<unsigned> u;
    std::optional<long long> i;
    auto result = checked_add(u,
                              i);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<long long>>);
    CHECK_FALSE(result);
    result = checked_add(i,
                         u);
    CHECK_FALSE(result);
  }
  SECTION("Both optional, one empty") {
    std::optional<unsigned> u;
    std::optional<long long> i(5);
    auto result = checked_add(u,
                              i);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<long long>>);
    CHECK_FALSE(result);
    result = checked_add(i,
                         u);
    CHECK_FALSE(result);
  }
  SECTION("Both optional, none empty") {
    std::optional<unsigned> u(1);
    std::optional<long long> i(5);
    auto result = checked_add(u,
                              i);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<long long>>);
    REQUIRE(result);
    CHECK(*result == 6);
    result = checked_add(i,
                         u);
    REQUIRE(result);
    CHECK(*result == 6);
  }
  SECTION("One optional, empty") {
    int i = -1;
    std::optional<int> o;
    auto result = checked_add(i,
                              o);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<int>>);
    CHECK_FALSE(result);
    result = checked_add(o,
                         i);
    CHECK_FALSE(result);
  }
  SECTION("One optional, not empty") {
    int i = -1;
    std::optional<int> o(-1);
    auto result = checked_add(i,
                              o);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<int>>);
    REQUIRE(result);
    CHECK(*result == -2);
    result = checked_add(o,
                         i);
    REQUIRE(result);
    CHECK(*result == -2);
  }
}

TEST_CASE("checked_multiply (integers)") {
  SECTION("No overflow, 2 operands") {
    int i = 2;
    unsigned u = 4;
    auto result = checked_multiply(i,
                                   u);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned>>);
    REQUIRE(result);
    CHECK(*result == 8);
  }
  SECTION("No overflow, 3 operands") {
    int i = 2;
    unsigned u = 4;
    long long l = 5;
    auto result = checked_multiply(i,
                                   u,
                                   l);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<long long>>);
    REQUIRE(result);
    CHECK(*result == 40);
  }
  SECTION("Overflow, smallest signed") {
    int a = std::numeric_limits<int>::min();
    int b = -1;
    auto result = checked_multiply(a,
                                   b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<int>>);
    CHECK_FALSE(result);
    result = checked_multiply(b,
                              a);
    CHECK_FALSE(result);
  }
  SECTION("No overflow, smallest signed") {
    int a = std::numeric_limits<int>::min();
    int b = 1;
    auto result = checked_multiply(a,
                                   b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<int>>);
    REQUIRE(result);
    CHECK(*result == std::numeric_limits<int>::min());
    result = checked_multiply(b,
                              a);
    REQUIRE(result);
    CHECK(*result == std::numeric_limits<int>::min());
    b = 0;
    result = checked_multiply(b,
                              a);
    REQUIRE(result);
    CHECK(*result == 0);
    result = checked_multiply(a,
                              b);
    REQUIRE(result);
    CHECK(*result == 0);
  }
  SECTION("Overflow, both signed") {
    int a = std::numeric_limits<int>::max();
    a /= 2;
    ++a;
    int b = 2;
    auto result = checked_multiply(a,
                                   b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<int>>);
    CHECK_FALSE(result);
    result = checked_multiply(b,
                              a);
  }
  SECTION("Overflow, mixed") {
    int a = std::numeric_limits<int>::max();
    a /= 2;
    ++a;
    unsigned b = 4;
    auto result = checked_multiply(a,
                                   b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned>>);
    CHECK_FALSE(result);
    result = checked_multiply(b,
                              a);
    CHECK_FALSE(result);
  }
  SECTION("No overflow, mixed") {
    int a = std::numeric_limits<int>::max();
    a /= 2;
    ++a;
    unsigned b = 3;
    auto result = checked_multiply(a,
                                   b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned>>);
    REQUIRE(result);
    CHECK(*result == (unsigned(a) * 3U));
    result = checked_multiply(b,
                              a);
    REQUIRE(result);
    CHECK(*result == (unsigned(a) * 3U));
  }
}

TEST_CASE("checked_multiply (optional)") {
  SECTION("Both optional, both empty") {
    std::optional<unsigned> a;
    std::optional<int> b;
    auto result = checked_multiply(a,
                                   b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned>>);
    CHECK_FALSE(result);
    result = checked_multiply(b,
                              a);
    CHECK_FALSE(result);
  }
  SECTION("Both optional, one empty") {
    std::optional<unsigned> a(5);
    std::optional<int> b;
    auto result = checked_multiply(a,
                                   b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned>>);
    CHECK_FALSE(result);
    result = checked_multiply(b,
                              a);
    CHECK_FALSE(result);
  }
  SECTION("Both optional, neither empty") {
    std::optional<unsigned> a(5);
    std::optional<int> b(3);
    auto result = checked_multiply(a,
                                   b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned>>);
    REQUIRE(result);
    CHECK(*result == 15);
    result = checked_multiply(b,
                              a);
    REQUIRE(result);
    CHECK(*result == 15);
  }
  SECTION("One optional and empty") {
    std::optional<unsigned> a;
    int b = 3;
    auto result = checked_multiply(a,
                                   b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned>>);
    CHECK_FALSE(result);
    result = checked_multiply(b,
                              a);
    CHECK_FALSE(result);
  }
  SECTION("One optional and not empty") {
    std::optional<unsigned> a(5);
    int b = 3;
    auto result = checked_multiply(a,
                                   b);
    static_assert(std::is_same_v<decltype(result),
                                 std::optional<unsigned>>);
    REQUIRE(result);
    CHECK(*result == 15);
    result = checked_multiply(b,
                              a);
    REQUIRE(result);
    CHECK(*result == 15);
  }
}

}
}
