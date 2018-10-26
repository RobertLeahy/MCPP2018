#include <mcpp/protocol/async_read.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/core/noncopyable.hpp>
#include <mcpp/system_error.hpp>
#include <mcpp/test/buffer_async_read_stream.hpp>
#include <mcpp/test/handler.hpp>

#include <catch2/catch.hpp>

namespace mcpp::protocol {
namespace {

using vector_type = std::vector<std::byte>;
using dynamic_buffer_type = boost::asio::dynamic_vector_buffer<vector_type::value_type,
                                                               vector_type::allocator_type>;
using dynamic_buffer_iterator_type = boost::asio::buffers_iterator<dynamic_buffer_type::const_buffers_type,
                                                                   std::byte>;

class after_read_length_initiating_function_state : private boost::noncopyable {
public:
  after_read_length_initiating_function_state() noexcept
    : invoked          (false),
      bytes_transferred(0),
      length           (0)
  {}
  void clear() noexcept {
    invoked = false;
    bytes_transferred = 0;
    length = 0;
    ec.clear();
    vec.clear();
  }
  bool            invoked;
  std::size_t     bytes_transferred;
  std::uint32_t   length;
  std::error_code ec;
  vector_type     vec;
};

class after_read_length_initiating_function {
public:
  explicit after_read_length_initiating_function(after_read_length_initiating_function_state& state) noexcept
    : state_(&state)
  {
    assert(state_);
  }
  template<typename CompletionHandler>
  void operator()(std::size_t bytes_transferred,
                  std::uint32_t length,
                  dynamic_buffer_type buffer,
                  CompletionHandler handler)
  {
    assert(state_);
    if (state_->invoked) {
      throw std::logic_error("Called multiple times");
    }
    state_->invoked = true;
    state_->bytes_transferred = bytes_transferred;
    state_->length = length;
    auto buffers = buffer.data();
    std::copy(dynamic_buffer_iterator_type::begin(buffers),
              dynamic_buffer_iterator_type::end(buffers),
              std::back_inserter(state_->vec));
    handler(state_->ec,
            std::move(buffer));
  }
private:
  after_read_length_initiating_function_state* state_;
};

class dynamic_buffer_handler_state : public test::read_handler_state {
public:
  void clear() noexcept {
    test::read_handler_state::clear();
    dynamic_buffer.reset();
  }
  std::optional<dynamic_buffer_type> dynamic_buffer;
};

class dynamic_buffer_handler : public test::read_handler {
public:
  explicit dynamic_buffer_handler(dynamic_buffer_handler_state& state) noexcept
    : test::read_handler(state),
      state_            (&state)
  {}
  void operator()(std::error_code ec,
                  std::size_t bytes_transferred,
                  dynamic_buffer_type buffer)
  {
    assert(state_);
    static_cast<test::read_handler&>(*this)(ec,
                                            bytes_transferred);
    state_->dynamic_buffer.emplace(std::move(buffer));
  }
private:
  dynamic_buffer_handler_state* state_;
};

TEST_CASE("async_read (with intermediate initiating function)",
          "[mcpp][async][protocol][read]")
{
  dynamic_buffer_handler_state state;
  after_read_length_initiating_function_state after_read_state;
  after_read_length_initiating_function after_read(after_read_state);
  vector_type in;
  vector_type out;
  dynamic_buffer_type buffer(out);
  boost::asio::io_context ioc;
  test::buffer_async_read_stream stream(ioc.get_executor());
  SECTION("Immediate end of file") {
    protocol::async_read(stream,
                         std::move(buffer),
                         after_read,
                         test::read_handler(state));
    CHECK_FALSE(after_read_state.invoked);
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers);
    CHECK_FALSE(after_read_state.invoked);
    CHECK(state.invoked);
    CHECK(state.ec);
    CHECK(state.ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition());
  }
  SECTION("Empty") {
    in.push_back(std::byte{0});
    stream.buffer(in.data(),
                  in.size());
    protocol::async_read(stream,
                         std::move(buffer),
                         after_read,
                         test::read_handler(state));
    CHECK_FALSE(after_read_state.invoked);
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers);
    CHECK(after_read_state.invoked);
    CHECK(state.invoked);
    REQUIRE_FALSE(state.ec);
    CHECK(state.bytes_transferred == 1);
    CHECK(stream.read() == 1);
    REQUIRE(out.size() == 1);
    CHECK(out.front() == std::byte{0});
    CHECK(after_read_state.bytes_transferred == 1);
    CHECK(after_read_state.length == 0);
    REQUIRE(after_read_state.vec.size() == 1);
    CHECK(after_read_state.vec.front() == std::byte{0});
  }
  SECTION("Multiple") {
    in.push_back(std::byte{1});
    in.push_back(std::byte{5});
    in.push_back(std::byte{2});
    in.push_back(std::byte{6});
    in.push_back(std::byte{7});
    stream.buffer(in.data(),
                  in.size());
    dynamic_buffer_handler handler(state);
    protocol::async_read(stream,
                         std::move(buffer),
                         after_read,
                         handler);
    CHECK_FALSE(after_read_state.invoked);
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers);
    CHECK(after_read_state.invoked);
    CHECK(state.invoked);
    REQUIRE_FALSE(state.ec);
    CHECK(state.bytes_transferred == 2);
    CHECK(stream.read() == 2);
    REQUIRE(out.size() == 2);
    CHECK(out[0] == std::byte{1});
    CHECK(out[1] == std::byte{5});
    CHECK(after_read_state.bytes_transferred == 1);
    CHECK(after_read_state.length == 1);
    REQUIRE(after_read_state.vec.size() == 1);
    CHECK(after_read_state.vec.front() == std::byte{1});
    REQUIRE(state.dynamic_buffer);
    auto buffers = state.dynamic_buffer->data();
    CHECK(std::equal(dynamic_buffer_iterator_type::begin(buffers),
                     dynamic_buffer_iterator_type::end(buffers),
                     out.begin(),
                     out.end()));
    auto b = std::move(*state.dynamic_buffer);
    state.clear();
    after_read_state.clear();
    ioc.reset();
    protocol::async_read(stream,
                         std::move(b),
                         after_read,
                         handler);
    CHECK_FALSE(after_read_state.invoked);
    CHECK_FALSE(state.invoked);
    handlers = ioc.poll();
    CHECK(handlers);
    CHECK(after_read_state.invoked);
    CHECK(state.invoked);
    REQUIRE_FALSE(state.ec);
    CHECK(state.bytes_transferred == 3);
    CHECK(stream.read() == 5);
    REQUIRE(out.size() == 5);
    CHECK(out[2] == std::byte{2});
    CHECK(out[3] == std::byte{6});
    CHECK(out[4] == std::byte{7});
    CHECK(after_read_state.bytes_transferred == 1);
    CHECK(after_read_state.length == 2);
    REQUIRE(after_read_state.vec.size() == 3);
    CHECK(after_read_state.vec[0] == std::byte{1});
    CHECK(after_read_state.vec[1] == std::byte{5});
    CHECK(after_read_state.vec[2] == std::byte{2});
    REQUIRE(state.dynamic_buffer);
    buffers = state.dynamic_buffer->data();
    CHECK(std::equal(dynamic_buffer_iterator_type::begin(buffers),
                     dynamic_buffer_iterator_type::end(buffers),
                     out.begin(),
                     out.end()));
    auto b2 = std::move(*state.dynamic_buffer);
    state.clear();
    after_read_state.clear();
    ioc.reset();
    protocol::async_read(stream,
                         std::move(b2),
                         after_read,
                         handler);
    CHECK_FALSE(after_read_state.invoked);
    CHECK_FALSE(state.invoked);
    handlers = ioc.poll();
    CHECK(handlers);
    CHECK_FALSE(after_read_state.invoked);
    CHECK(state.invoked);
    CHECK(state.ec);
    CHECK(state.ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition());
    CHECK(state.bytes_transferred == 0);
    CHECK(stream.read() == 5);
    REQUIRE(out.size() == 5);
    REQUIRE(state.dynamic_buffer);
    buffers = state.dynamic_buffer->data();
    CHECK(std::equal(dynamic_buffer_iterator_type::begin(buffers),
                     dynamic_buffer_iterator_type::end(buffers),
                     out.begin(),
                     out.end()));
  }
}

TEST_CASE("async_read (no intermediate initiating function)",
          "[mcpp][async][protocol][read]")
{
  dynamic_buffer_handler_state state;
  vector_type in;
  vector_type out;
  dynamic_buffer_type buffer(out);
  boost::asio::io_context ioc;
  test::buffer_async_read_stream stream(ioc.get_executor());
  SECTION("Multiple") {
    in.push_back(std::byte{1});
    in.push_back(std::byte{5});
    in.push_back(std::byte{2});
    in.push_back(std::byte{6});
    in.push_back(std::byte{7});
    stream.buffer(in.data(),
                  in.size());
    dynamic_buffer_handler handler(state);
    protocol::async_read(stream,
                         std::move(buffer),
                         handler);
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers);
    CHECK(state.invoked);
    REQUIRE_FALSE(state.ec);
    CHECK(state.bytes_transferred == 2);
    CHECK(stream.read() == 2);
    REQUIRE(out.size() == 1);
    CHECK(out[0] == std::byte{5});
    REQUIRE(state.dynamic_buffer);
    auto buffers = state.dynamic_buffer->data();
    CHECK(std::equal(dynamic_buffer_iterator_type::begin(buffers),
                     dynamic_buffer_iterator_type::end(buffers),
                     out.begin(),
                     out.end()));
    auto b = std::move(*state.dynamic_buffer);
    state.clear();
    ioc.reset();
    protocol::async_read(stream,
                         std::move(b),
                         handler);
    handlers = ioc.poll();
    CHECK(handlers);
    CHECK(state.invoked);
    REQUIRE_FALSE(state.ec);
    CHECK(state.bytes_transferred == 3);
    CHECK(stream.read() == 5);
    REQUIRE(out.size() == 3);
    CHECK(out[1] == std::byte{6});
    CHECK(out[2] == std::byte{7});
    REQUIRE(state.dynamic_buffer);
    buffers = state.dynamic_buffer->data();
    CHECK(std::equal(dynamic_buffer_iterator_type::begin(buffers),
                     dynamic_buffer_iterator_type::end(buffers),
                     out.begin(),
                     out.end()));
    auto b2 = std::move(*state.dynamic_buffer);
    state.clear();
    ioc.reset();
    protocol::async_read(stream,
                         std::move(b2),
                         handler);
    CHECK_FALSE(state.invoked);
    handlers = ioc.poll();
    CHECK(handlers);
    CHECK(state.invoked);
    CHECK(state.ec);
    CHECK(state.ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition());
    CHECK(state.bytes_transferred == 0);
    CHECK(stream.read() == 5);
    REQUIRE(out.size() == 3);
    REQUIRE(state.dynamic_buffer);
    buffers = state.dynamic_buffer->data();
    CHECK(std::equal(dynamic_buffer_iterator_type::begin(buffers),
                     dynamic_buffer_iterator_type::end(buffers),
                     out.begin(),
                     out.end()));
  }
}

TEST_CASE("limit_after_read_length_initiating_function") {
  bool invoked = false;
  std::error_code ec;
  vector_type vec;
  vec.push_back(std::byte{0});
  vec.push_back(std::byte{1});
  vec.push_back(std::byte{2});
  vec.push_back(std::byte{3});
  vec.push_back(std::byte{4});
  vec.push_back(std::byte{5});
  dynamic_buffer_type buffer(vec);
  std::optional<dynamic_buffer_type> out_buffer;
  limit_after_read_length_initiating_function f(5);
  auto handler = [&](auto lec,
                     auto lbuffer)
  {
    if (invoked) {
      throw std::runtime_error("Already invoked");
    }
    invoked = true;
    ec = lec;
    out_buffer.emplace(std::move(lbuffer));
  };
  SECTION("Pass") {
    f(1,
      5,
      std::move(buffer),
      handler);
    CHECK(invoked);
    CHECK_FALSE(ec);
  }
  SECTION("Fail") {
    f(1,
      6,
      std::move(buffer),
      handler);
    CHECK(invoked);
    CHECK(ec);
  }
  REQUIRE(out_buffer);
  auto buffers = out_buffer->data();
  CHECK(std::equal(dynamic_buffer_iterator_type::begin(buffers),
                   dynamic_buffer_iterator_type::end(buffers),
                   vec.begin(),
                   vec.end()));
}

}
}
