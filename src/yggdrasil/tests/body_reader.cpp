#include <mcpp/yggdrasil/body_reader.hpp>

#include <cstddef>
#include <sstream>
#include <string_view>
#include <variant>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <mcpp/test/buffer_async_read_stream.hpp>
#include <mcpp/test/handler.hpp>
#include <mcpp/yggdrasil/authenticate.hpp>
#include <mcpp/yggdrasil/body.hpp>
#include <mcpp/yggdrasil/error.hpp>

#include <catch2/catch.hpp>

namespace mcpp::yggdrasil::tests {
namespace {

TEST_CASE("body_reader (request)",
          "[mcpp][yggdrasil][body][body_reader][beast]")
{
  test::read_handler_state state;
  std::vector<std::byte> buffer;
  boost::asio::dynamic_vector_buffer dynamic_buffer(buffer);
  using request_body_type = request_body<authenticate_response,
                                         authenticate_response_parser>;
  using request_type = boost::beast::http::request<request_body_type>;
  request_type request;
  boost::asio::io_context ioc;
  test::buffer_async_read_stream stream(ioc.get_executor());
  std::ostringstream ss;
  SECTION("Correct body") {
    std::string_view text("{\"accessToken\":\"foo\","
                           "\"clientToken\":\"bar\","
                           "\"availableProfiles\":[{\"id\":\"barbaz\","
                                                   "\"name\":\"quuxcorge\"}],"
                           "\"selectedProfile\":{\"id\":\"quux\","
                                                "\"name\":\"baz\"},"
                           "\"user\":{\"id\":\"corge\","
                                     "\"properties\":[]}}");
    ss << "POST / HTTP/1.1\r\n"
          "Content-Type: application/json\r\n"
          "Content-Length: "
       << text.size() << "\r\n"
                         "\r\n"
       << text;
    auto str = ss.str();
    INFO(str);
    stream.buffer(reinterpret_cast<const std::byte*>(str.data()),
                  str.size());
    boost::beast::http::async_read(stream,
                                   dynamic_buffer,
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
    CHECK(state.bytes_transferred == str.size());
    CHECK(request.body().access_token == "foo");
    CHECK(request.body().client_token == "bar");
    REQUIRE(request.body().available_profiles);
    auto iter = request.body().available_profiles->begin();
    REQUIRE_FALSE(iter == request.body().available_profiles->end());
    CHECK(iter->id == "barbaz");
    CHECK(iter->name == "quuxcorge");
    CHECK_FALSE(iter->legacy);
    ++iter;
    CHECK(iter == request.body().available_profiles->end());
    REQUIRE(request.body().selected_profile);
    CHECK(request.body().selected_profile->id == "quux");
    CHECK(request.body().selected_profile->name == "baz");
    CHECK_FALSE(request.body().selected_profile->legacy);
    REQUIRE(request.body().user);
    CHECK(request.body().user->id == "corge");
    CHECK(request.body().user->properties.empty());
  }
  SECTION("No body") {
    std::string_view text("POST / HTTP/1.1\r\n"
                          "Content-Type: application/json\r\n"
                          "\r\n");
    stream.buffer(reinterpret_cast<const std::byte*>(text.data()),
                  text.size());
    boost::beast::http::async_read(stream,
                                   dynamic_buffer,
                                   request,
                                   test::read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    CHECK(state.ec);
  }
  SECTION("Empty body") {
    std::string_view text("POST / HTTP/1.1\r\n"
                          "Content-Type: application/json\r\n"
                          "Content-Length: 0\r\n"
                          "\r\n");
    stream.buffer(reinterpret_cast<const std::byte*>(text.data()),
                  text.size());
    boost::beast::http::async_read(stream,
                                   dynamic_buffer,
                                   request,
                                   test::read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    CHECK(state.ec);
  }
}

TEST_CASE("body_reader (response)",
          "[mcpp][yggdrasil][body][body_reader][beast]")
{
  test::read_handler_state state;
  std::vector<std::byte> buffer;
  boost::asio::dynamic_vector_buffer dynamic_buffer(buffer);
  using response_body_type = response_body<authenticate_response,
                                           authenticate_response_parser>;
  using response_type = boost::beast::http::response<response_body_type>;
  response_type response;
  boost::asio::io_context ioc;
  test::buffer_async_read_stream stream(ioc.get_executor());
  std::ostringstream ss;
  SECTION("Correct body") {
    std::string_view text("{\"accessToken\":\"foo\","
                           "\"clientToken\":\"bar\","
                           "\"availableProfiles\":[{\"id\":\"barbaz\","
                                                   "\"name\":\"quuxcorge\"}],"
                           "\"selectedProfile\":{\"id\":\"quux\","
                                                "\"name\":\"baz\"},"
                           "\"user\":{\"id\":\"corge\","
                                     "\"properties\":[]}}");
    ss << "HTTP/1.1 200 OK\r\n"
          "Content-Type: application/json\r\n"
          "Content-Length: "
       << text.size() << "\r\n"
                         "\r\n"
       << text;
    auto str = ss.str();
    INFO(str);
    stream.buffer(reinterpret_cast<const std::byte*>(str.data()),
                  str.size());
    boost::beast::http::async_read(stream,
                                   dynamic_buffer,
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
    CHECK(state.bytes_transferred == str.size());
    auto&& auth = std::get<authenticate_response>(response.body());
    CHECK(auth.access_token == "foo");
    CHECK(auth.client_token == "bar");
    REQUIRE(auth.available_profiles);
    auto iter = auth.available_profiles->begin();
    REQUIRE_FALSE(iter == auth.available_profiles->end());
    CHECK(iter->id == "barbaz");
    CHECK(iter->name == "quuxcorge");
    CHECK_FALSE(iter->legacy);
    ++iter;
    CHECK(iter == auth.available_profiles->end());
    REQUIRE(auth.selected_profile);
    CHECK(auth.selected_profile->id == "quux");
    CHECK(auth.selected_profile->name == "baz");
    CHECK_FALSE(auth.selected_profile->legacy);
    REQUIRE(auth.user);
    CHECK(auth.user->id == "corge");
    CHECK(auth.user->properties.empty());
  }
  SECTION("Error") {
    std::string_view text("{\"error\":\"test error\","
                           "\"errorMessage\":\"test error message\"}");
    ss << "HTTP/1.1 404 Not Found\r\n"
          "Content-Type: application/json\r\n"
          "Content-Length: "
       << text.size() << "\r\n"
                         "\r\n"
       << text;
    auto str = ss.str();
    INFO(str);
    stream.buffer(reinterpret_cast<const std::byte*>(str.data()),
                  str.size());
    boost::beast::http::async_read(stream,
                                   dynamic_buffer,
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
    CHECK(state.bytes_transferred == str.size());
    auto&& err = std::get<error>(response.body());
    CHECK(err.error == "test error");
    CHECK(err.error_message == "test error message");
    CHECK_FALSE(err.cause);
  }
  SECTION("No body") {
    std::string_view text("HTTP/1.1 200 OK\r\n"
                          "Content-Type: application/json\r\n"
                          "\r\n");
    stream.buffer(reinterpret_cast<const std::byte*>(text.data()),
                  text.size());
    boost::beast::http::async_read(stream,
                                   dynamic_buffer,
                                   response,
                                   test::read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    CHECK(state.ec);
  }
  SECTION("Empty body") {
    std::string_view text("HTTP/1.1 200 OK\r\n"
                          "Content-Type: application/json\r\n"
                          "Content-Length: 0\r\n"
                          "\r\n");
    stream.buffer(reinterpret_cast<const std::byte*>(text.data()),
                  text.size());
    boost::beast::http::async_read(stream,
                                   dynamic_buffer,
                                   response,
                                   test::read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ioc.run();
    CHECK_FALSE(handlers == 0);
    CHECK(state.invoked);
    CHECK(state.ec);
  }
}

}
}
