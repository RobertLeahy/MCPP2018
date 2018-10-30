#include <mcpp/zlib/inflate.hpp>

#include <cstddef>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <mcpp/zlib/inflate_stream.hpp>

#include <catch2/catch.hpp>

namespace mcpp::zlib::tests {
namespace {

TEST_CASE("inflate",
          "[mcpp][zlib][inflate]")
{
  inflate_stream stream(15 + 32);
  std::vector<std::byte> in;
  //  This was made with gzip
  in.push_back(std::byte{0b00011111});
  in.push_back(std::byte{0b10001011});
  in.push_back(std::byte{0b00001000});
  in.push_back(std::byte{0b00001000});
  in.push_back(std::byte{0b10100100});
  in.push_back(std::byte{0b10111011});
  in.push_back(std::byte{0b11010111});
  in.push_back(std::byte{0b01011011});
  in.push_back(std::byte{0b00000000});
  in.push_back(std::byte{0b00000011});
  in.push_back(std::byte{0b01101000});
  in.push_back(std::byte{0b01100101});
  in.push_back(std::byte{0b01101100});
  in.push_back(std::byte{0b01101100});
  in.push_back(std::byte{0b01101111});
  in.push_back(std::byte{0b00101110});
  in.push_back(std::byte{0b01110100});
  in.push_back(std::byte{0b01111000});
  in.push_back(std::byte{0b01110100});
  in.push_back(std::byte{0b00000000});
  in.push_back(std::byte{0b11110011});
  in.push_back(std::byte{0b01001000});
  in.push_back(std::byte{0b11001101});
  in.push_back(std::byte{0b11001001});
  in.push_back(std::byte{0b11001001});
  in.push_back(std::byte{0b11100111});
  in.push_back(std::byte{0b00000010});
  in.push_back(std::byte{0b00000000});
  in.push_back(std::byte{0b00010110});
  in.push_back(std::byte{0b00110101});
  in.push_back(std::byte{0b10010110});
  in.push_back(std::byte{0b00110001});
  in.push_back(std::byte{0b00000110});
  in.push_back(std::byte{0b00000000});
  in.push_back(std::byte{0b00000000});
  in.push_back(std::byte{0b00000000});
  std::string out;
  boost::asio::dynamic_string_buffer<std::string::value_type,
                                     std::string::traits_type,
                                     std::string::allocator_type> buffer(out);
  std::error_code ec;
  SECTION("Simple") {
    auto pair = inflate(stream,
                        boost::asio::buffer(in),
                        std::move(buffer),
                        ec);
    CHECK_FALSE(ec);
    CHECK(pair.first == in.size());
    CHECK(pair.second.size() == 6);
    CHECK(out == "Hello\n");
  }
  SECTION("Input not all consumed") {
    in.push_back(std::byte{0});
    auto pair = inflate(stream,
                        boost::asio::buffer(in),
                        std::move(buffer),
                        ec);
    CHECK_FALSE(ec);
    CHECK((in.size() - 1) == pair.first);
    CHECK(pair.second.size() == 6);
    CHECK(out == "Hello\n");
  }
  SECTION("Output too small") {
    boost::asio::dynamic_string_buffer<std::string::value_type,
                                       std::string::traits_type,
                                       std::string::allocator_type> buffer(out,
                                                                           5);
    auto pair = inflate(stream,
                        boost::asio::buffer(in),
                        std::move(buffer),
                        ec);
    CHECK(ec);
    CHECK(ec.default_error_condition() == make_error_code(std::errc::not_enough_memory).default_error_condition());
    CHECK(in.size() != pair.first);
    CHECK(pair.second.size() == 5);
    CHECK(out == "Hello");
  }
  SECTION("Corrupt") {
    in[0] = std::byte{0};
    inflate(stream,
            boost::asio::buffer(in),
            std::move(buffer),
            ec);
    CHECK(ec);
  }
  SECTION("Missing trailing bytes") {
    auto before = in.size();
    while (!in.empty()) {
      in.pop_back();
      INFO("Missing " << (before - in.size()) << " bytes (size is " << in.size() << ')');
      auto pair = inflate(stream,
                          boost::asio::buffer(in),
                          std::move(buffer),
                          ec);
      CHECK(ec);
      CHECK(pair.first == in.size());
    }
  }
}

}
}
