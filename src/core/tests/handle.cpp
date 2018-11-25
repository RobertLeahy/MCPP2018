#include <mcpp/handle.hpp>

#include <cassert>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/core/noncopyable.hpp>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

class state : private boost::noncopyable {
public:
  state()
    : destroy_count(0),
      valid        (0),
      invalid      (0)
  {}
  std::vector<int*> create;
  std::vector<int*> destroy;
  std::size_t       destroy_count;
  std::size_t       valid;
  std::size_t       invalid;
};

class policy : public pointer_handle_policy_base<int*> {
private:
  using base = pointer_handle_policy_base<int*>;
public:
  explicit policy(state& s) noexcept
    : state_(&s)
  {
    assert(state_);
  }
  bool valid(const_native_handle_type handle) const noexcept {
    assert(state_);
    ++state_->valid;
    return base::valid(handle);
  }
  native_handle_type invalid() const noexcept {
    ++state_->invalid;
    return base::invalid();
  }
  native_handle_type create() {
    if (state_->create.empty()) {
      throw std::bad_alloc();
    }
    native_handle_type retr = state_->create.front();
    state_->create.erase(state_->create.begin());
    return retr;
  }
  void destroy(native_handle_type handle) noexcept {
    ++state_->destroy_count;
    try {
      state_->destroy.push_back(handle);
    } catch (...) {}
  }
private:
  state* state_;
};

static_assert(std::is_same_v<int*,
                             policy::native_handle_type>);
static_assert(std::is_same_v<const int*,
                             policy::const_native_handle_type>);

using handle_type = handle<policy>;

static_assert(std::is_same_v<int*,
                             handle_type::native_handle_type>);
static_assert(std::is_same_v<const int*,
                             handle_type::const_native_handle_type>);

TEST_CASE("handle default ctor",
          "[mcpp][core][handle]")
{
  state s;
  policy p(s);
  int i = 5;
  s.create.push_back(&i);
  {
    handle h(p);
    CHECK(h.native_handle() == &i);
    CHECK(std::as_const(h).native_handle() == &i);
    CHECK(s.create.empty());
    CHECK(s.destroy.empty());
    CHECK(s.destroy_count == 0);
    //  We don't assert invalid because the handle
    //  is allowed to call that "spuriously" for
    //  the sake of debug assertions
    CHECK(s.invalid == 0);
  }
  CHECK(s.create.empty());
  CHECK(s.destroy_count == 1);
  CHECK(s.valid != 0);
  CHECK(s.invalid == 0);
  REQUIRE(s.destroy.size() == 1);
  CHECK(s.destroy.front() == &i);
}

TEST_CASE("handle unary ctor",
          "[mcpp][core][handle]")
{
  state s;
  policy p(s);
  int i = 5;
  {
    handle h(&i,
             p);
    CHECK(h.native_handle() == &i);
    CHECK(std::as_const(h).native_handle() == &i);
    CHECK(s.create.empty());
    CHECK(s.destroy.empty());
    CHECK(s.destroy_count == 0);
    //  We don't assert invalid because the handle
    //  is allowed to call that "spuriously" for
    //  the sake of debug assertions
    CHECK(s.invalid == 0);
  }
  CHECK(s.create.empty());
  CHECK(s.destroy_count == 1);
  CHECK(s.valid != 0);
  CHECK(s.invalid == 0);
  REQUIRE(s.destroy.size() == 1);
  CHECK(s.destroy.front() == &i);
}

TEST_CASE("handle move ctor",
          "[mcpp][core][handle]")
{
  state s;
  policy p(s);
  int i = 5;
  {
    handle h(&i,
             p);
    handle m(std::move(h));
    CHECK(m.native_handle() == &i);
    CHECK(std::as_const(m).native_handle() == &i);
    CHECK(s.create.empty());
    CHECK(s.destroy.empty());
    CHECK(s.destroy_count == 0);
    //  We don't assert invalid because the handle
    //  is allowed to call that "spuriously" for
    //  the sake of debug assertions
    CHECK(s.invalid != 0);
  }
  CHECK(s.create.empty());
  CHECK(s.destroy_count == 1);
  CHECK(s.valid != 0);
  CHECK(s.invalid != 0);
  REQUIRE(s.destroy.size() == 1);
  CHECK(s.destroy.front() == &i);
}

TEST_CASE("handle move assignment operator",
          "[mcpp][core][handle]")
{
  state s;
  policy p(s);
  int i = 5;
  int n = 6;
  {
    handle h(&i,
             p);
    handle m(&n,
             p);
    m = std::move(h);
    CHECK(m.native_handle() == &i);
    CHECK(std::as_const(m).native_handle() == &i);
    CHECK(s.create.empty());
    CHECK(s.destroy_count == 1);
    CHECK(s.valid != 0);
    CHECK(s.invalid != 0);
    REQUIRE(s.destroy.size() == 1);
    CHECK(s.destroy.back() == &n);
  }
  CHECK(s.create.empty());
  CHECK(s.destroy_count == 2);
  CHECK(s.valid != 0);
  CHECK(s.invalid != 0);
  REQUIRE(s.destroy.size() == 2);
  CHECK(s.destroy.back() == &i);
}

}
}
