#include <mcpp/protocol/decompress.hpp>

#include <algorithm>
#include <cstddef>
#include <system_error>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <mcpp/zlib/deflate.hpp>
#include <mcpp/zlib/deflate_stream.hpp>
#include <mcpp/zlib/inflate_stream.hpp>

#include <catch2/catch.hpp>

namespace mcpp::protocol::tests {
namespace {

TEST_CASE("decompress",
          "[mcpp][decompress][protocol]")
{
  using vector_type = std::vector<std::byte>;
  using dynamic_buffer_type = boost::asio::dynamic_vector_buffer<vector_type::value_type,
                                                                 vector_type::allocator_type>;
  vector_type out;
  dynamic_buffer_type buffer(out);
  vector_type in;
  bool invoked = false;
  std::size_t size = 0;
  bool retr = true;
  auto f = [&](auto s) noexcept {
    invoked = true;
    size = s;
    return retr;
  };
  zlib::inflate_stream inflate;
  std::error_code ec;
  SECTION("False predicate") {
    retr = false;
    in.push_back(std::byte{5});
    auto b = decompress(inflate,
                        boost::asio::buffer(in),
                        std::move(buffer),
                        f,
                        ec);
    CHECK_FALSE(ec);
    CHECK(invoked);
    CHECK(b.second.data().size() == 0);
    CHECK(size == 5);
    CHECK(boost::asio::buffer_size(b.first) == 0);
  }
  SECTION("Negative length") {
    in.push_back(std::byte{0b11111111});
    in.push_back(std::byte{0b11111111});
    in.push_back(std::byte{0b11111111});
    in.push_back(std::byte{0b11111111});
    in.push_back(std::byte{0b00001111});
    auto b = decompress(inflate,
                        boost::asio::buffer(in),
                        std::move(buffer),
                        f,
                        ec);
    CHECK(ec);
    CHECK(ec.default_error_condition() == make_error_code(std::errc::value_too_large).default_error_condition());
    CHECK_FALSE(invoked);
    CHECK(b.second.data().size() == 0);
    CHECK(boost::asio::buffer_size(b.first) == 0);
  }
  SECTION("Success") {
    vector_type vec;
    vec.push_back(std::byte{5});
    dynamic_buffer_type compressed(vec);
    in.push_back(std::byte{0});
    in.push_back(std::byte{1});
    in.push_back(std::byte{2});
    in.push_back(std::byte{3});
    in.push_back(std::byte{4});
    zlib::deflate_stream deflate;
    auto pair = zlib::deflate(deflate,
                              boost::asio::buffer(in),
                              std::move(compressed),
                              ec);
    REQUIRE_FALSE(ec);
    REQUIRE(pair.first == in.size());
    auto b = decompress(inflate,
                        boost::asio::buffer(vec),
                        std::move(buffer),
                        f,
                        ec);
    CHECK_FALSE(ec);
    CHECK(invoked);
    CHECK(b.second.data().size() == 5);
    CHECK(size == 5);
    CHECK(std::equal(out.begin(),
                     out.end(),
                     in.begin(),
                     in.end()));
    CHECK(boost::asio::buffer_size(b.first) == 0);
  }
  SECTION("Padded") {
    vector_type vec;
    vec.push_back(std::byte{5});
    dynamic_buffer_type compressed(vec);
    in.push_back(std::byte{0});
    in.push_back(std::byte{1});
    in.push_back(std::byte{2});
    in.push_back(std::byte{3});
    in.push_back(std::byte{4});
    zlib::deflate_stream deflate;
    auto pair = zlib::deflate(deflate,
                              boost::asio::buffer(in),
                              std::move(compressed),
                              ec);
    REQUIRE_FALSE(ec);
    REQUIRE(pair.first == in.size());
    vec.push_back(std::byte{0});
    auto b = decompress(inflate,
                        boost::asio::buffer(vec),
                        std::move(buffer),
                        f,
                        ec);
    CHECK(ec);
    CHECK(invoked);
    CHECK(size == 5);
    CHECK(boost::asio::buffer_size(b.first) == 1);
  }
  SECTION("Length too long") {
    vector_type vec;
    vec.push_back(std::byte{6});
    dynamic_buffer_type compressed(vec);
    in.push_back(std::byte{0});
    in.push_back(std::byte{1});
    in.push_back(std::byte{2});
    in.push_back(std::byte{3});
    in.push_back(std::byte{4});
    zlib::deflate_stream deflate;
    auto pair = zlib::deflate(deflate,
                              boost::asio::buffer(in),
                              std::move(compressed),
                              ec);
    REQUIRE_FALSE(ec);
    REQUIRE(pair.first == in.size());
    auto b = decompress(inflate,
                        boost::asio::buffer(vec),
                        std::move(buffer),
                        f,
                        ec);
    CHECK(ec);
    CHECK(invoked);
    CHECK(size == 6);
    CHECK(boost::asio::buffer_size(b.first) == 0);
  }
  SECTION("Length too short") {
    vector_type vec;
    vec.push_back(std::byte{4});
    dynamic_buffer_type compressed(vec);
    in.push_back(std::byte{0});
    in.push_back(std::byte{1});
    in.push_back(std::byte{2});
    in.push_back(std::byte{3});
    in.push_back(std::byte{4});
    zlib::deflate_stream deflate;
    auto pair = zlib::deflate(deflate,
                              boost::asio::buffer(in),
                              std::move(compressed),
                              ec);
    REQUIRE_FALSE(ec);
    REQUIRE(pair.first == in.size());
    auto b = decompress(inflate,
                        boost::asio::buffer(vec),
                        std::move(buffer),
                        f,
                        ec);
    CHECK(ec);
    CHECK(invoked);
    CHECK(size == 4);
    CHECK(boost::asio::buffer_size(b.first) == 0);
  }
}

}
}
