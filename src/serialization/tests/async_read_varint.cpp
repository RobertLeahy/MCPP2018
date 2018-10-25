#include <mcpp/serialization/async_read_varint.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <mcpp/system_error.hpp>
#include <mcpp/test/buffer_async_read_stream.hpp>
#include <mcpp/test/handler.hpp>

#include <catch2/catch.hpp>

namespace mcpp::serialization::tests {
namespace {

using vector_type = std::vector<std::byte>;
using dynamic_buffer_type = boost::asio::dynamic_vector_buffer<vector_type::value_type,
                                                               vector_type::allocator_type>;

template<typename Integer>
class handler_state : public test::read_handler_state {
public:
  handler_state() noexcept
    : i(0)
  {}
  void clear() noexcept {
    test::read_handler_state::clear();
    i = 0;
    dynamic_buffer = std::nullopt;
  }
  Integer                            i;
  std::optional<dynamic_buffer_type> dynamic_buffer;
};

template<typename Integer>
class handler : public test::read_handler {
public:
  explicit handler(handler_state<Integer>& state) noexcept
    : test::read_handler(state),
      state_            (&state)
  {
    assert(state_);
  }
  void operator()(std::error_code ec,
                  std::size_t bytes_transferred,
                  Integer i)
  {
    assert(state_);
    static_cast<test::read_handler&>(*this)(ec,
                                            bytes_transferred);
    state_->i = i;
  }
private:
  handler_state<Integer>* state_;
};

template<typename Integer>
class dynamic_buffer_handler : public handler<Integer> {
public:
  explicit dynamic_buffer_handler(handler_state<Integer>& state) noexcept
    : handler<Integer>(state),
      state_          (&state)
  {
    assert(state_);
  }
  void operator()(std::error_code ec,
                  std::size_t bytes_transferred,
                  Integer i,
                  dynamic_buffer_type dynamic_buffer)
  {
    assert(state_);
    static_cast<handler<Integer>&>(*this)(ec,
                                          bytes_transferred,
                                          i);
    state_->dynamic_buffer.emplace(std::move(dynamic_buffer));
  }
private:
  handler_state<Integer>* state_;
};

TEST_CASE("async_read_varint",
          "[mcpp][serialization][varint][async]")
{
  handler_state<std::int32_t> state;
  using vector_type = std::vector<std::byte>;
  vector_type buffer;
  vector_type scratch;
  boost::asio::dynamic_vector_buffer<vector_type::value_type,
                                     vector_type::allocator_type> dynamic_buffer(scratch);
  boost::asio::io_context ioc;
  test::buffer_async_read_stream stream(ioc.get_executor());
  SECTION("Single byte (extra)") {
    buffer.push_back(std::byte{1});
    //  This shouldn't be read
    buffer.push_back(std::byte{5});
    stream.buffer(buffer.data(),
                  buffer.size());
    async_read_varint<std::int32_t>(stream,
                                    std::move(dynamic_buffer),
                                    handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    REQUIRE_FALSE(state.ec);
    CHECK(state.bytes_transferred == 1);
    CHECK(state.i == 1);
    CHECK(stream.read() == 1);
    REQUIRE(scratch.size() == 1);
    CHECK(scratch[0] == buffer[0]);
  }
  SECTION("Multiple bytes (extra)") {
    buffer.push_back(std::byte{0b10101100});
    buffer.push_back(std::byte{0b00000010});
    //  This shouldn't be read
    buffer.push_back(std::byte{5});
    stream.buffer(buffer.data(),
                  buffer.size());
    async_read_varint<std::int32_t>(stream,
                                    std::move(dynamic_buffer),
                                    handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    REQUIRE_FALSE(state.ec);
    CHECK(state.bytes_transferred == 2);
    CHECK(state.i == 300);
    CHECK(stream.read() == 2);
  }
  SECTION("End of file") {
    buffer.push_back(std::byte{0b10101100});
    stream.buffer(buffer.data(),
                  buffer.size());
    async_read_varint<std::int32_t>(stream,
                                    std::move(dynamic_buffer),
                                    handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    CHECK(state.ec);
    CHECK(state.bytes_transferred == 1);
    CHECK(stream.read() == 1);
    CHECK(state.ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition());
  }
  SECTION("Receive DynamicBuffer") {
    buffer.push_back(std::byte{0b11111111});
    buffer.push_back(std::byte{0b11111111});
    buffer.push_back(std::byte{0b11111111});
    buffer.push_back(std::byte{0b11111111});
    buffer.push_back(std::byte{0b00001111});
    buffer.push_back(std::byte{0b10101100});
    buffer.push_back(std::byte{0b00000010});
    stream.buffer(buffer.data(),
                  buffer.size());
    async_read_varint<std::int32_t>(stream,
                                    std::move(dynamic_buffer),
                                    dynamic_buffer_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    REQUIRE_FALSE(state.ec);
    CHECK(state.bytes_transferred == 5);
    CHECK(state.i == -1);
    CHECK(stream.read() == 5);
    REQUIRE(state.dynamic_buffer);
    auto in = state.dynamic_buffer->data();
    using iterator_type = boost::asio::buffers_iterator<decltype(in),
                                                        std::byte>;
    CHECK(std::equal(iterator_type::begin(in),
                     iterator_type::end(in),
                     buffer.begin(),
                     buffer.begin() + 5));
    ioc.reset();
    state.clear();
    async_read_varint<std::int32_t>(stream,
                                    std::move(*state.dynamic_buffer),
                                    dynamic_buffer_handler(state));
    CHECK_FALSE(state.invoked);
    handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    REQUIRE_FALSE(state.ec);
    CHECK(state.bytes_transferred == 2);
    CHECK(state.i == 300);
    CHECK(stream.read() == 7);
    REQUIRE(state.dynamic_buffer);
    in = state.dynamic_buffer->data();
    CHECK(std::equal(iterator_type::begin(in),
                     iterator_type::end(in),
                     buffer.begin(),
                     buffer.begin() + 7));
    ioc.reset();
    state.clear();
    async_read_varint<std::int32_t>(stream,
                                    std::move(*state.dynamic_buffer),
                                    dynamic_buffer_handler(state));
    CHECK_FALSE(state.invoked);
    handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    CHECK(state.ec);
    CHECK(state.ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition());
    CHECK(state.bytes_transferred == 0);
    CHECK(stream.read() == 7);
    REQUIRE(state.dynamic_buffer);
    in = state.dynamic_buffer->data();
    CHECK(std::equal(iterator_type::begin(in),
                     iterator_type::end(in),
                     buffer.begin(),
                     buffer.begin() + 7));
  }
}

}
}
