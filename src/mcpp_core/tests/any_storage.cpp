#include <mcpp/any_storage.hpp>

#include <utility>
#include <mcpp/test/object.hpp>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

TEST_CASE("any_storage default constructor",
          "[mcpp][any_storage][core]")
{
  any_storage storage;
  CHECK_FALSE(storage.capacity());
  CHECK_FALSE(storage);
  CHECK_FALSE(storage.has_value());
}

TEST_CASE("any_storage move constructor",
          "[mcpp][any_storage][core]")
{
  test::object::state state;
  any_storage a;
  a.emplace<test::object>(state);
  any_storage b(std::move(a));
  CHECK_FALSE(state.destruct);
  CHECK(b);
}

TEST_CASE("any_storage move assignment operator",
          "[mcpp][any_storage][core]")
{
  test::object::state state;
  any_storage a;
  a.emplace<test::object>(state);
  any_storage b;
  SECTION("Move assign empty") {
    b = std::move(a);
    CHECK_FALSE(state.destruct);
    CHECK(b);
  }
  SECTION("Move assign non-empty") {
    b.emplace<test::object>(state);
    b = std::move(a);
    CHECK(state.destruct == 1);
    CHECK(b);
  }
}

TEST_CASE("any_storage destructor",
          "[mcpp][any_storage][core]")
{
  test::object::state state;
  {
    any_storage storage;
    storage.emplace<test::object>(state);
  }
  CHECK(state.destruct);
}

TEST_CASE("any_storage::reset"
          "[mcpp][any_storage][core]")
{
  test::object::state state;
  any_storage storage;
  storage.emplace<test::object>(state);
  storage.reset();
  CHECK(state.destruct);
}

TEST_CASE("any_storage::reserve",
          "[mcpp][any_storage][core]")
{
  test::object::state state;
  any_storage storage;
  SECTION("Increase storage") {
    storage.reserve(5);
    CHECK(storage.capacity() >= 5);
  }
  SECTION("Increase storage with object") {
    storage.emplace<test::object>(state);
    storage.reserve(5);
    CHECK(storage.capacity() >= 5);
    CHECK(state.destruct);
  }
  SECTION("Don't increase storage with object") {
    storage.emplace<test::object>(state);
    storage.reserve(storage.capacity());
    CHECK(state.destruct);
  }
}

TEST_CASE("any_storage::emplace",
          "[mcpp][any_storage][core]")
{
  test::object::state state;
  any_storage storage;
  SECTION("Create") {
    int& i = storage.emplace<int>(5);
    CHECK(i == 5);
  }
  SECTION("Replace") {
    storage.emplace<test::object>(state);
    storage.emplace<test::object>(state);
    CHECK(state.construct == 2);
    CHECK(state.destruct == 1);
  }
}

}
}
