#include <mcpp/completion_handler_base.hpp>

#include <memory>
#include <type_traits>
#include <utility>
#include <boost/asio/associated_allocator.hpp>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/system_executor.hpp>
#include <mcpp/test/allocator.hpp>
#include <mcpp/test/handler.hpp>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

template<typename Executor,
         typename CompletionHandler>
class completion_handler : public completion_handler_base<Executor,
                                                          CompletionHandler>
{
private:
  using base = completion_handler_base<Executor,
                                       CompletionHandler>;
public:
  using base::base;
  void operator()() {
    base::invoke();
  }
};

template<typename Executor>
class executor_handler : public test::handler {
public:
  executor_handler(test::handler_state& state,
                   const Executor& ex) noexcept
    : test::handler(state),
      ex_          (ex)
  {}
  using executor_type = Executor;
  auto get_executor() const noexcept{
    return ex_;
  }
private:
  Executor ex_;
};

template<typename Handler,
         typename Executor,
         typename ExpectedExecutor>
constexpr bool executor_v = std::is_same_v<boost::asio::associated_executor_t<completion_handler_base<Executor,
                                                                                                      Handler>>,
                                           ExpectedExecutor>;

static_assert(executor_v<test::handler,
                         boost::asio::io_context::executor_type,
                         boost::asio::io_context::executor_type>);
static_assert(executor_v<executor_handler<boost::asio::system_executor>,
                         boost::asio::io_context::executor_type,
                         boost::asio::system_executor>);

TEST_CASE("completion_handler_base::get_executor") {
  test::handler_state state;
  boost::asio::io_context a;
  boost::asio::io_context b;
  SECTION("No association") {
    completion_handler<boost::asio::io_context::executor_type,
                       test::handler> handler(a.get_executor(),
                                              test::handler(state));
    boost::asio::post(std::move(handler));
    CHECK_FALSE(state.invoked);
    auto handlers = b.poll();
    CHECK(handlers == 0);
    CHECK_FALSE(state.invoked);
    handlers = a.poll();
    CHECK(handlers == 1);
    CHECK(state.invoked);
  }
  SECTION("Association") {
    using handler_type = executor_handler<boost::asio::io_context::executor_type>;
    completion_handler<boost::asio::io_context::executor_type,
                       handler_type> handler(a.get_executor(),
                                             handler_type(state,
                                                          b.get_executor()));
    boost::asio::post(std::move(handler));
    CHECK_FALSE(state.invoked);
    auto handlers = a.poll();
    CHECK(handlers == 0);
    CHECK_FALSE(state.invoked);
    handlers = b.poll();
    CHECK(handlers == 1);
    CHECK(state.invoked);
  }
}

template<typename Allocator>
class allocator_handler : public test::handler {
public:
  allocator_handler(test::handler_state& state,
                    const Allocator& alloc) noexcept
    : test::handler(state),
      alloc_       (alloc)
  {}
  using allocator_type = Allocator;
  auto get_allocator() const noexcept {
    return alloc_;
  }
private:
  Allocator alloc_;
};

template<typename Handler,
         typename ExpectedAllocator>
constexpr bool allocator_v = std::is_same_v<boost::asio::associated_allocator_t<completion_handler_base<boost::asio::io_context::executor_type,
                                                                                                        Handler>>,
                                            ExpectedAllocator>;

static_assert(allocator_v<test::handler,
                          std::allocator<void>>);
static_assert(allocator_v<allocator_handler<test::allocator<std::allocator<void>>>,
                          test::allocator<std::allocator<void>>>);

TEST_CASE("completion_handler_base::get_allocator") {
  test::handler_state state;
  test::allocator_state state_a;
  using allocator_type = test::allocator<int>;
  allocator_type a(state_a);
  test::allocator_state state_b;
  allocator_type b(state_b);
  boost::asio::io_context ioc;
  SECTION("Association") {
    using handler_type = allocator_handler<allocator_type>;
    completion_handler<boost::asio::io_context::executor_type,
                       handler_type> handler(ioc.get_executor(),
                                             handler_type(state,
                                                          a));
    auto proto = boost::asio::get_associated_allocator(handler,
                                                       b);
    std::allocator_traits<decltype(proto)>::rebind_alloc<int> alloc(proto);
    using traits_type = std::allocator_traits<decltype(alloc)>;
    traits_type::deallocate(alloc,
                            traits_type::allocate(alloc,
                                                  1),
                            1);
    CHECK(state_a.allocate == 1);
    CHECK(state_b.allocate == 0);
  }
  SECTION("No association") {
    completion_handler<boost::asio::io_context::executor_type,
                       test::handler> handler(ioc.get_executor(),
                                              test::handler(state));
    auto proto = boost::asio::get_associated_allocator(handler,
                                                       a);
    std::allocator_traits<decltype(proto)>::rebind_alloc<int> alloc(proto);
    using traits_type = std::allocator_traits<decltype(alloc)>;
    traits_type::deallocate(alloc,
                            traits_type::allocate(alloc,
                                                  1),
                            1);
    CHECK(state_a.allocate == 0);
    CHECK(state_b.allocate == 0);
  }
}

}
}
