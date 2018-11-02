#include <mcpp/serialization/async_write_varint.hpp>

#include <cstddef>
#include <optional>
#include <system_error>
#include <utility>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <mcpp/system_error.hpp>
#include <mcpp/test/buffer_async_write_stream.hpp>
#include <mcpp/test/handler.hpp>

#include <catch2/catch.hpp>

namespace mcpp::serialization::tests {
namespace {

using vector_type = std::vector<std::byte>;
using dynamic_buffer_type = boost::asio::dynamic_vector_buffer<vector_type::value_type,
                                                               vector_type::allocator_type>;

class write_handler_state : public test::write_handler_state {
public:
  void clear() noexcept {
    test::write_handler_state::clear();
    dynamic_buffer = std::nullopt;
  }
  std::optional<dynamic_buffer_type> dynamic_buffer;
};

class write_handler : public test::write_handler {
public:
  explicit write_handler(write_handler_state& state) noexcept
    : test::write_handler(state),
      state_             (&state)
  {}
  void operator()(std::error_code ec,
                  std::size_t bytes_transferred,
                  dynamic_buffer_type dynamic_buffer)
  {
    static_cast<test::write_handler&>(*this)(ec,
                                             bytes_transferred);
    state_->dynamic_buffer.emplace(std::move(dynamic_buffer));
  }
private:
  write_handler_state* state_;
};

TEST_CASE("async_write_varint",
          "[mcpp][serialization][async][write][varint]")
{
  std::byte out[16];
  vector_type vec;
  dynamic_buffer_type dynamic_buffer(vec);
  test::write_handler_state state;
  boost::asio::io_context ioc;
  test::buffer_async_write_stream stream(ioc.get_executor(),
                                         out,
                                         sizeof(out));
  SECTION("Single byte") {
    async_write_varint(stream,
                       5,
                       std::move(dynamic_buffer),
                       test::write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    CHECK_FALSE(state.ec);
    REQUIRE(state.bytes_transferred == 1);
    CHECK(out[0] == std::byte{5});
    CHECK(stream.written() == 1);
    REQUIRE(vec.size() == 1);
    CHECK(vec[0] == std::byte{5});
  }
  SECTION("Multiple bytes") {
    async_write_varint(stream,
                       300,
                       std::move(dynamic_buffer),
                       test::write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    CHECK_FALSE(state.ec);
    REQUIRE(state.bytes_transferred == 2);
    CHECK(out[0] == std::byte{0b10101100});
    CHECK(out[1] == std::byte{0b00000010});
    CHECK(stream.written() == 2);
    REQUIRE(vec.size() == 2);
    CHECK(vec[0] == std::byte{0b10101100});
    CHECK(vec[1] == std::byte{0b00000010});
  }
  SECTION("Error") {
    //  This will cause EOF
    stream.buffer(out,
                  1);
    async_write_varint(stream,
                       300,
                       std::move(dynamic_buffer),
                       test::write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    CHECK(state.ec);
    CHECK(is_eof(state.ec));
    REQUIRE(state.bytes_transferred == 1);
    CHECK(out[0] == std::byte{0b10101100});
    CHECK(stream.written() == 1);
    REQUIRE(vec.size() == 1);
    CHECK(vec[0] == std::byte{0b10101100});
  }
  SECTION("Return DynamicBuffer") {
    write_handler_state state;
    async_write_varint(stream,
                       300,
                       std::move(dynamic_buffer),
                       write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    CHECK_FALSE(state.ec);
    REQUIRE(state.bytes_transferred == 2);
    CHECK(out[0] == std::byte{0b10101100});
    CHECK(out[1] == std::byte{0b00000010});
    CHECK(stream.written() == 2);
    REQUIRE(vec.size() == 2);
    CHECK(vec[0] == std::byte{0b10101100});
    CHECK(vec[1] == std::byte{0b00000010});
    REQUIRE(state.dynamic_buffer);
    CHECK(state.dynamic_buffer->size() == 2);
  }
}

}
}
