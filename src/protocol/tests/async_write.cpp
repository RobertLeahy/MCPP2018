#include <mcpp/protocol/async_write.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <mcpp/checked.hpp>
#include <mcpp/system_error.hpp>
#include <mcpp/test/buffer_async_write_stream.hpp>
#include <mcpp/test/handler.hpp>

#include <catch2/catch.hpp>

namespace mcpp::protocol::tests {
namespace {

using vector_type = std::vector<std::byte>;
using dynamic_buffer_type = boost::asio::dynamic_vector_buffer<vector_type::value_type,
                                                               vector_type::allocator_type>;

TEST_CASE("async_write",
          "[mcpp][async][protocol][write]")
{
  std::byte buffer[16];
  std::vector<std::byte> in;
  test::write_handler_state state;
  boost::asio::io_context ioc;
  test::buffer_async_write_stream stream(ioc.get_executor());
  vector_type scratch;
  dynamic_buffer_type dynamic_buffer(scratch);
  SECTION("Empty") {
    stream.buffer(buffer,
                  sizeof(buffer));
    protocol::async_write(stream,
                          std::move(dynamic_buffer),
                          boost::asio::buffer(in),
                          test::write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    CHECK_FALSE(state.ec);
    CHECK(state.bytes_transferred == 1);
    REQUIRE(scratch.size() == 1);
    CHECK(scratch[0] == std::byte{0});
    REQUIRE(stream.written() == 1);
    CHECK(buffer[0] == std::byte{0});
  }
  SECTION("Non-empty") {
    stream.buffer(buffer,
                  sizeof(buffer));
    in.push_back(std::byte{5});
    in.push_back(std::byte{6});
    protocol::async_write(stream,
                          std::move(dynamic_buffer),
                          boost::asio::buffer(in),
                          test::write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    CHECK_FALSE(state.ec);
    CHECK(state.bytes_transferred == 3);
    REQUIRE(scratch.size() == 1);
    CHECK(scratch[0] == std::byte{2});
    REQUIRE(stream.written() == 3);
    CHECK(buffer[0] == std::byte{2});
    CHECK(buffer[1] == std::byte{5});
    CHECK(buffer[2] == std::byte{6});
  }
  SECTION("Error") {
    stream.buffer(buffer,
                  1);
    in.push_back(std::byte{5});
    in.push_back(std::byte{6});
    protocol::async_write(stream,
                          std::move(dynamic_buffer),
                          boost::asio::buffer(in),
                          test::write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers != 0);
    REQUIRE(state.invoked);
    CHECK(state.ec);
    CHECK(state.ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition());
    CHECK(state.bytes_transferred == 1);
    REQUIRE(scratch.size() == 1);
    CHECK(scratch[0] == std::byte{2});
    REQUIRE(stream.written() == 1);
    CHECK(buffer[0] == std::byte{2});
  }
  SECTION("Overflow") {
    auto size = std::numeric_limits<std::uint32_t>::max();
    ++size;
    auto in_size = checked_cast<std::size_t>(size);
    if (!in_size) {
      WARN("std::size_t cannot represent " << size);
    } else {
      //  I can't think of a better way to do this
      boost::asio::const_buffer buffer(nullptr,
                                       *in_size);
      protocol::async_write(stream,
                            std::move(dynamic_buffer),
                            buffer,
                            test::write_handler(state));
      CHECK_FALSE(state.invoked);
      auto handlers = ioc.poll();
      CHECK(handlers != 0);
      REQUIRE(state.invoked);
      CHECK(state.ec);
      CHECK(stream.written() == 0);
    }
  }
}

}
}
