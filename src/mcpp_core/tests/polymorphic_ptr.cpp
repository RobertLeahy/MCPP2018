#include <mcpp/polymorphic_ptr.hpp>

#include <string_view>
#include <utility>
#include <boost/core/noncopyable.hpp>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

class a : private boost::noncopyable {
public:
  virtual ~a() noexcept {}
  virtual std::string_view get() const noexcept {
    return "a";
  }
};
class b : public a {
public:
  virtual std::string_view get() const noexcept override {
    return "b";
  }
};
class c : public b {
public:
  virtual std::string_view get() const noexcept override {
    return "c";
  }
};

TEST_CASE("polymorphic_ptr converting move constructor",
          "[mcpp][polymorphic_ptr][core]")
{
  polymorphic_ptr<b> b_ptr;
  SECTION("Null") {
    polymorphic_ptr<a> a_ptr(std::move(b_ptr));
    CHECK_FALSE(a_ptr);
  }
  SECTION("Not null") {
    b_ptr.emplace<c>();
    polymorphic_ptr<a> a_ptr(std::move(b_ptr));
    CHECK(a_ptr);
  }
}

TEST_CASE("polymorphic_ptr converting move assignment operator",
          "[mcpp][polymorphic_ptr][core]")
{
  polymorphic_ptr<b> b_ptr;
  polymorphic_ptr<a> a_ptr;
  SECTION("Null") {
    a_ptr = std::move(b_ptr);
    CHECK_FALSE(a_ptr);
  }
  SECTION("Not null") {
    b_ptr.emplace<c>();
    a_ptr = std::move(b_ptr);
    CHECK(a_ptr);
  }
}

TEST_CASE("polymorphic_ptr dereference & ::get",
          "[mcpp][polymorphic_ptr][core]")
{
  polymorphic_ptr<a> ptr;
  ptr.emplace<c>();
  SECTION("*") {
    CHECK((*ptr).get() == "c");
  }
  SECTION("->") {
    CHECK(ptr->get() == "c");
  }
  SECTION("get") {
    a* p = ptr.get();
    REQUIRE(p);
    CHECK(p->get() == "c");
  }
}

}
}
