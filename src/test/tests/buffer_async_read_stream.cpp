#include <mcpp/test/buffer_async_read_stream.hpp>

#include <array>
#include <cstddef>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <mcpp/system_error.hpp>
#include <mcpp/test/handler.hpp>

#include <catch2/catch.hpp>

namespace mcpp::test::tests {
namespace {

TEST_CASE("buffer_async_read_stream::get_executor",
          "[mcpp][buffer_async_read_stream][async][test]")
{
  handler_state state;
  boost::asio::io_context ioc;
  buffer_async_read_stream stream(ioc.get_executor());
  boost::asio::post(stream.get_executor(),
                    handler(state));
  auto handlers = ioc.poll();
  CHECK(handlers);
  CHECK(state.invoked);
}

TEST_CASE("buffer_async_read_stream::async_read_some",
          "[mcpp][buffer_async_read_stream][async][test]")
{
  std::vector<std::byte> vec;
  vec.push_back(std::byte{1});
  vec.push_back(std::byte{2});
  std::array<std::byte,
             4> buffer;
  read_handler_state state;
  boost::asio::io_context ioc;
  buffer_async_read_stream stream(ioc.get_executor(),
                                  vec.data(),
                                  vec.size());
  SECTION("All") {
    stream.async_read_some(boost::asio::buffer(buffer),
                           read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers);
    REQUIRE(state.invoked);
    REQUIRE_FALSE(state.ec);
    REQUIRE(state.bytes_transferred == 2);
    CHECK(buffer[0] == std::byte{1});
    CHECK(buffer[1] == std::byte{2});
    CHECK(stream.read() == 2);
  }
  SECTION("Some") {
    stream.async_read_some(boost::asio::buffer(buffer.data(),
                                               1),
                           read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers);
    REQUIRE(state.invoked);
    REQUIRE_FALSE(state.ec);
    REQUIRE(state.bytes_transferred == 1);
    CHECK(buffer[0] == std::byte{1});
    CHECK(stream.read() == 1);
    state.clear();
    ioc.reset();
    stream.async_read_some(boost::asio::buffer(buffer.data() + 1,
                                               buffer.size() - 1),
                           read_handler(state));
    CHECK_FALSE(state.invoked);
    handlers = ioc.poll();
    CHECK(handlers);
    REQUIRE(state.invoked);
    REQUIRE_FALSE(state.ec);
    REQUIRE(state.bytes_transferred == 1);
    CHECK(buffer[1] == std::byte{2});
    CHECK(stream.read() == 2);
  }
  SECTION("Empty buffer => non-empty buffer") {
    stream.buffer(nullptr,
                  0);
    stream.async_read_some(boost::asio::buffer(buffer),
                           read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers);
    REQUIRE(state.invoked);
    CHECK(state.ec);
    CHECK(state.bytes_transferred == 0);
    CHECK(is_eof(state.ec));
    CHECK(stream.read() == 0);
  }
  SECTION("Empty buffer => empty buffer") {
    stream.buffer(nullptr,
                  0);
    stream.async_read_some(boost::asio::buffer(buffer.data(),
                                               0),
                           read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.poll();
    CHECK(handlers);
    REQUIRE(state.invoked);
    REQUIRE_FALSE(state.ec);
    CHECK(state.bytes_transferred == 0);
    CHECK(stream.read() == 0);
  }
}

}
}
