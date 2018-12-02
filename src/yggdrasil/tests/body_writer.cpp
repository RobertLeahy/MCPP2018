#include <mcpp/yggdrasil/body_writer.hpp>

#include <cstddef>
#include <ios>
#include <string_view>
#include <boost/asio/io_context.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <mcpp/test/buffer_async_write_stream.hpp>
#include <mcpp/test/handler.hpp>
#include <mcpp/yggdrasil/authenticate.hpp>
#include <mcpp/yggdrasil/body.hpp>
#include <mcpp/yggdrasil/error.hpp>

#include <catch2/catch.hpp>

namespace mcpp::yggdrasil::tests {
namespace {

TEST_CASE("body_writer (request)",
          "[mcpp][yggdrasil][body][body_writer][beast]")
{
  test::write_handler_state state;
  using request_body_type = request_body<authenticate_response,
                                         authenticate_response_parser>;
  using request_type = boost::beast::http::request<request_body_type>;
  request_type request;
  request.target("/");
  request.method(boost::beast::http::verb::post);
  request.set("Content-Type",
              "application/json");
  request.body().access_token = "foo";
  request.body().client_token = "bar";
  request.prepare_payload();
  std::byte buffer[1024];
  boost::asio::io_context ioc;
  test::buffer_async_write_stream stream(ioc.get_executor());
  stream.buffer(buffer,
                sizeof(buffer));
  boost::beast::http::async_write(stream,
                                  request,
                                  test::read_handler(state));
  CHECK_FALSE(state.invoked);
  auto handlers = ioc.run();
  CHECK_FALSE(handlers == 0);
  CHECK(state.invoked);
  {
    INFO(state.ec.message());
    CHECK_FALSE(state.ec);
  }
  std::string_view body("{\"accessToken\":\"foo\","
                         "\"clientToken\":\"bar\"}");
  std::ostringstream ss;
  ss << "POST / HTTP/1.1\r\n"
        "Content-Type: application/json\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
     << std::hex << body.size() << "\r\n"
     << body << "\r\n"
     << "0\r\n"
        "\r\n";
  auto expected = ss.str();
  CHECK(state.bytes_transferred == expected.size());
  std::string_view sv(reinterpret_cast<const char*>(buffer),
                      stream.written());
  CHECK(sv == expected);
}

TEST_CASE("body_writer (response)",
          "[mcpp][yggdrasil][body][body_writer][beast]")
{
  test::write_handler_state state;
  using response_body_type = response_body<authenticate_response,
                                           authenticate_response_parser>;
  using response_type = boost::beast::http::response<response_body_type>;
  response_type response;
  response.set("Content-Type",
               "application/json");
  std::byte buffer[1024];
  boost::asio::io_context ioc;
  test::buffer_async_write_stream stream(ioc.get_executor());
  stream.buffer(buffer,
                sizeof(buffer));
  SECTION("Not an error") {
    response.result(boost::beast::http::status::ok);
    auto&& auth = response.body().emplace<authenticate_response>();
    auth.access_token = "foo";
    auth.client_token = "bar";
    response.prepare_payload();
    boost::beast::http::async_write(stream,
                                    response,
                                    test::read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    {
      INFO(state.ec.message());
      CHECK_FALSE(state.ec);
    }
    std::string_view body("{\"accessToken\":\"foo\","
                           "\"clientToken\":\"bar\"}");
    std::ostringstream ss;
    ss << "HTTP/1.1 200 OK\r\n"
          "Content-Type: application/json\r\n"
          "Transfer-Encoding: chunked\r\n"
          "\r\n"
       << std::hex << body.size() << "\r\n"
       << body << "\r\n"
       << "0\r\n"
          "\r\n";
    auto expected = ss.str();
    CHECK(state.bytes_transferred == expected.size());
    std::string_view sv(reinterpret_cast<const char*>(buffer),
                        stream.written());
    CHECK(sv == expected);
  }
  SECTION("Error") {
    response.result(boost::beast::http::status::not_found);
    auto&& err = response.body().emplace<error>();
    err.error = "foo";
    err.error_message = "bar";
    response.prepare_payload();
    boost::beast::http::async_write(stream,
                                    response,
                                    test::read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    {
      INFO(state.ec.message());
      CHECK_FALSE(state.ec);
    }
    std::string_view body("{\"error\":\"foo\","
                           "\"errorMessage\":\"bar\"}");
    std::ostringstream ss;
    ss << "HTTP/1.1 404 Not Found\r\n"
          "Content-Type: application/json\r\n"
          "Transfer-Encoding: chunked\r\n"
          "\r\n"
       << std::hex << body.size() << "\r\n"
       << body << "\r\n"
       << "0\r\n"
          "\r\n";
    auto expected = ss.str();
    CHECK(state.bytes_transferred == expected.size());
    std::string_view sv(reinterpret_cast<const char*>(buffer),
                        stream.written());
    CHECK(sv == expected);
  }
}

}
}
