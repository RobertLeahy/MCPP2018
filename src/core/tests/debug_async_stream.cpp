#include <mcpp/debug_async_stream.hpp>

#include <cstddef>
#include <sstream>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <mcpp/system_error.hpp>
#include <mcpp/test/buffer_async_read_stream.hpp>
#include <mcpp/test/buffer_async_write_stream.hpp>
#include <mcpp/test/handler.hpp>

#include <catch2/catch.hpp>

namespace mcpp::tests {
namespace {

TEST_CASE("debug_async_stream::async_read_some",
          "[mcpp][core][async][debug][debug_async_stream]")
{
  std::vector<std::byte> in;
  test::read_handler_state state;
  std::ostringstream ss;
  std::byte out[16]; 
  boost::asio::io_context ioc;
  test::buffer_async_read_stream underlying(ioc.get_executor());
  debug_async_stream_settings settings;
  settings.name = "Test";
  SECTION("Empty read") {
    debug_async_stream stream(underlying,
                              settings,
                              ss);
    stream.async_read_some(boost::asio::buffer(out),
                           test::read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    {
      INFO(state.ec.message());
      CHECK(is_eof(state.ec));
    }
    CHECK(state.bytes_transferred == 0);
    auto str = ss.str();
    CHECK("Test: Read (0 bytes)\n" == str);
  }
  SECTION("Non-empty read") {
    debug_async_stream stream(underlying,
                              settings,
                              ss);
    in.push_back(std::byte{0});
    in.push_back(std::byte{1});
    in.push_back(std::byte{2});
    in.push_back(std::byte{3});
    underlying.buffer(in.data(),
                      in.size());
    stream.async_read_some(boost::asio::buffer(out),
                           test::read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    {
      INFO(state.ec.message());
      CHECK_FALSE(state.ec);
    }
    CHECK(state.bytes_transferred == 4);
    auto str = ss.str();
    CHECK("Test: Read (4 bytes):\n"
          "00 01 02 03                                      ....\n"
    //     00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
          == str);
  }
  SECTION("text format") {
    settings.format = debug_async_stream_settings::output_format::text;
    debug_async_stream stream(underlying,
                              settings,
                              ss);
    in.push_back(std::byte{'h'});
    in.push_back(std::byte{'i'});
    underlying.buffer(in.data(),
                      in.size());
    stream.async_read_some(boost::asio::buffer(out),
                           test::read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    {
      INFO(state.ec.message());
      CHECK_FALSE(state.ec);
    }
    CHECK(state.bytes_transferred == 2);
    auto str = ss.str();
    CHECK("Test: Read (2 bytes):\n"
          "hi\n" == str);
  }
}

TEST_CASE("debug_async_stream::async_write_some",
          "[mcpp][core][async][debug][debug_async_stream]")
{
  std::byte out[5];
  test::write_handler_state state;
  std::ostringstream ss;
  boost::asio::io_context ioc;
  test::buffer_async_write_stream underlying(ioc.get_executor());
  underlying.buffer(out,
                    sizeof(out));
  debug_async_stream_settings settings;
  SECTION("hex_dump format") {
    debug_async_stream stream(underlying,
                              settings,
                              ss);
    const std::byte in[] = {std::byte{0}, std::byte{1}, std::byte{2}, std::byte{3}};
    stream.async_write_some(boost::asio::buffer(in),
                            test::write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    {
      INFO(state.ec.message());
      CHECK_FALSE(state.ec);
    }
    CHECK(state.bytes_transferred == 4);
    auto str = ss.str();
    CHECK("Write (4 bytes):\n"
          "00 01 02 03                                      ....\n"
    //     00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
          == str);
  }
  SECTION("text format") {
    settings.format = debug_async_stream_settings::output_format::text;
    debug_async_stream stream(underlying,
                              settings,
                              ss);
    const std::byte in[] = {std::byte{'h'}, std::byte{'i'}};
    stream.async_write_some(boost::asio::buffer(in),
                            test::write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    {
      INFO(state.ec.message());
      CHECK_FALSE(state.ec);
    }
    CHECK(state.bytes_transferred == 2);
    auto str = ss.str();
    CHECK("Write (2 bytes):\n"
          "hi\n" == str);
  }
  SECTION("Partial") {
    debug_async_stream stream(underlying,
                              settings,
                              ss);
    const std::byte in[] = {std::byte{0}, std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}};
    stream.async_write_some(boost::asio::buffer(in),
                            test::write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    {
      INFO(state.ec.message());
      CHECK_FALSE(state.ec);
    }
    CHECK(state.bytes_transferred == sizeof(out));
    auto str = ss.str();
    CHECK("Write (5 bytes):\n"
          "00 01 02 03 04                                   .....\n"
    //     00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
          == str);
  }
  SECTION("Partial (text)") {
    settings.format = debug_async_stream_settings::output_format::text;
    debug_async_stream stream(underlying,
                              settings,
                              ss);
    const std::byte in[] = {std::byte{'0'}, std::byte{'1'}, std::byte{'2'}, std::byte{'3'}, std::byte{'4'}, std::byte{'5'}};
    stream.async_write_some(boost::asio::buffer(in),
                            test::write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    {
      INFO(state.ec.message());
      CHECK_FALSE(state.ec);
    }
    CHECK(state.bytes_transferred == sizeof(out));
    auto str = ss.str();
    CHECK("Write (5 bytes):\n"
          "01234\n" == str);
  }
}

}
}
