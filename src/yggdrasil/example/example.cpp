#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <variant>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/asio/ssl/verify_mode.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <mcpp/debug_async_stream.hpp>
#include <mcpp/system_error.hpp>
#include <mcpp/yggdrasil/authenticate.hpp>
#include <mcpp/yggdrasil/body.hpp>
#include <mcpp/yggdrasil/error.hpp>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

namespace {

template<typename T>
void pretty_print(const T& message) {
  rapidjson::OStreamWrapper wrapper(std::cout);
  rapidjson::PrettyWriter<rapidjson::OStreamWrapper> w(wrapper);
  if (!mcpp::yggdrasil::to_json(message,
                                w))
  {
    throw std::runtime_error("RapidJSON failure");
  }
  std::cout << std::endl;
}

class visitor_base {
public:
  void operator()(const mcpp::yggdrasil::error& err) const {
    std::cout << "Error!\n";
    pretty_print(err);
  }
};

class response_visitor : public visitor_base {
public:
  using visitor_base::operator();
  void operator()(const mcpp::yggdrasil::authenticate_response& auth) const {
    std::cout << "Success!\n";
    pretty_print(auth);
  }
};

void Main(int argc,
          char** argv)
{
  if (argc != 3) {
    throw std::runtime_error("Requires exactly two arguments");
  }
  using request_body_type = mcpp::yggdrasil::request_body<mcpp::yggdrasil::authenticate_request,
                                                          mcpp::yggdrasil::authenticate_request_parser>;
  using request_type = boost::beast::http::request<request_body_type>;
  request_type request;
  request.method(boost::beast::http::verb::post);
  request.target("/authenticate");
  request.set("Content-Type",
              "application/json");
  request.set("Host",
              "authserver.mojang.com");
  request.body().agent.name = "Minecraft";
  request.body().username = argv[1];
  request.body().password = argv[2];
  request.body().request_user = true;
  request.prepare_payload();
  using response_body_type = mcpp::yggdrasil::response_body<mcpp::yggdrasil::authenticate_response,
                                                            mcpp::yggdrasil::authenticate_response_parser>;
  using response_type = boost::beast::http::response<response_body_type>;
  response_type response;
  std::vector<std::byte> buffer;
  boost::asio::dynamic_vector_buffer dynamic_buffer(buffer);
  boost::asio::io_context ioc;
  boost::asio::ip::tcp::socket socket(ioc);
  socket.open(boost::asio::ip::tcp::v4());
  mcpp::debug_async_stream_settings ciphertext_settings;
  ciphertext_settings.name = "Cipher Text";
  mcpp::debug_async_stream ciphertext_stream(socket,
                                             ciphertext_settings,
                                             std::cout);
  boost::asio::ssl::context ctx(boost::asio::ssl::context::tls);
  ctx.set_verify_mode(boost::asio::ssl::verify_none);
  boost::asio::ssl::stream<decltype(ciphertext_stream)&> stream(ciphertext_stream,
                                                                ctx);
  mcpp::debug_async_stream_settings plaintext_settings;
  plaintext_settings.format = mcpp::debug_async_stream_settings::output_format::text;
  plaintext_settings.name = "Plain Text";
  mcpp::debug_async_stream plaintext_stream(stream,
                                            plaintext_settings,
                                            std::cout);
  boost::asio::ip::tcp::endpoint ep;
  ep.address(boost::asio::ip::make_address("54.230.32.67"));
  ep.port(443);
  auto on_response = [&](auto ec,
                         auto bytes_transferred)
  {
    if (ec) {
      throw std::system_error(mcpp::to_error_code(ec));
    }
    std::cout << "Response received (status code " << response.result_int() << " " << response.reason() << ") (" << bytes_transferred << " bytes)" << std::endl;
    response_visitor visitor;
    std::visit(visitor,
               response.body());
  };
  auto on_request = [&](auto ec,
                        auto bytes_transferred)
  {
    if (ec) {
      throw std::system_error(mcpp::to_error_code(ec));
    }
    std::cout << "Request sent (" << bytes_transferred << " bytes)" << std::endl;
    boost::beast::http::async_read(plaintext_stream,
                                   dynamic_buffer,
                                   response,
                                   on_response);
  };
  auto on_handshake = [&](auto ec) {
    if (ec) {
      throw std::system_error(mcpp::to_error_code(ec));
    }
    std::cout << "TLS handshake complete" << std::endl;
    boost::beast::http::async_write(plaintext_stream,
                                    request,
                                    on_request);
  };
  auto on_connect = [&](auto ec) {
    if (ec) {
      throw std::system_error(mcpp::to_error_code(ec));
    }
    std::cout << "Connected to " << ep << std::endl;
    stream.async_handshake(boost::asio::ssl::stream_base::client,
                           on_handshake);
  };
  socket.async_connect(ep,
                       on_connect);
  auto handlers = ioc.run();
  std::cout << "Done (ran " << handlers << " handlers)" << std::endl;
}

}

int main(int argc,
         char** argv)
{
  try {
    try {
      Main(argc,
           argv);
    } catch (const std::exception& ex) {
      std::cerr << "ERROR: " << ex.what() << std::endl;
      throw;
    } catch (...) {
      std::cerr << "ERROR" << std::endl;
      throw;
    }
  } catch (...) {
    return EXIT_FAILURE;
  }
}
