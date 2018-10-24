#include <mcpp/allocate_unique.hpp>

#include <exception>
#include <memory>
#include <stdexcept>
#include <mcpp/test/allocator.hpp>
#include <mcpp/test/object.hpp>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

TEST_CASE("allocate_unique",
          "[mcpp][allocate_unique][memory][core]")
{
  test::allocator_state alloc_state;
  test::allocator<void> alloc(alloc_state);
  test::object::state state;
  SECTION("Naked pointer allocator") {
    auto ptr = allocate_unique<test::object>(alloc,
                                             state);
    CHECK(alloc_state.allocate == 1);
    CHECK(alloc_state.deallocate == 0);
    CHECK(state.construct == 1);
    CHECK(state.destruct == 0);
    ptr.reset();
    CHECK(alloc_state.deallocate == 1);
    CHECK(state.destruct == 1);
  }
  SECTION("Constructor throws") {
    state.construct_ex = std::make_exception_ptr(std::runtime_error("Test"));
    CHECK_THROWS_AS(allocate_unique<test::object>(alloc,
                                                  state),
                    std::runtime_error);
    CHECK(alloc_state.allocate == 1);
    CHECK(alloc_state.deallocate == 1);
  }
}

}
}
