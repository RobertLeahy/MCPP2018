#include <mcpp/protocol/compress.hpp>

#include <algorithm>
#include <cstddef>
#include <system_error>
#include <utility>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <mcpp/suffix_buffer_sequence.hpp>
#include <mcpp/zlib/deflate_stream.hpp>
#include <mcpp/zlib/inflate.hpp>
#include <mcpp/zlib/inflate_stream.hpp>

#include <catch2/catch.hpp>

namespace mcpp::protocol::tests {
namespace {

TEST_CASE("compress",
          "[mcpp][protocol][compress")
{
  std::vector<std::byte> in;
  zlib::deflate_stream deflate;
  std::vector<std::byte> out;
  boost::asio::dynamic_vector_buffer<std::byte,
                                     std::vector<std::byte>::allocator_type> buffer(out);
  std::error_code ec;
  zlib::inflate_stream inflate;
  std::vector<std::byte> inflated;
  boost::asio::dynamic_vector_buffer<std::byte,
                                     std::vector<std::byte>::allocator_type> inflated_buffer(inflated);
  SECTION("Empty") {
    auto b = protocol::compress(deflate,
                                boost::asio::buffer(in),
                                std::move(buffer),
                                ec);
    REQUIRE_FALSE(ec);
    REQUIRE(out.size() >= 1);
    CHECK(out.front() == std::byte{0});
    REQUIRE(b.size() >= 1);
    auto cb = b.data();
    using buffers_iterator_type = boost::asio::buffers_iterator<decltype(cb),
                                                                std::byte>;
    CHECK(std::equal(buffers_iterator_type::begin(cb),
                     buffers_iterator_type::end(cb),
                     out.begin(),
                     out.end()));
    mcpp::suffix_buffer_sequence sbs(cb,
                                     b.size() - 1);
    auto pair = zlib::inflate(inflate,
                              sbs,
                              std::move(inflated_buffer),
                              in.size(),
                              ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.first == boost::asio::buffer_size(sbs));
    CHECK(pair.second.size() == 0);
  }
  SECTION("Non-empty") {
    in.resize(300);
    unsigned char u(0);
    for (auto&& b : in) {
      b = std::byte{u++};
    }
    auto b = protocol::compress(deflate,
                                boost::asio::buffer(in),
                                std::move(buffer),
                                ec);
    REQUIRE_FALSE(ec);
    REQUIRE(out.size() >= 2);
    CHECK(out[0] == std::byte{0b10101100});
    CHECK(out[1] == std::byte{0b00000010});
    REQUIRE(b.size() >= 2);
    auto cb = b.data();
    using buffers_iterator_type = boost::asio::buffers_iterator<decltype(cb),
                                                                std::byte>;
    CHECK(std::equal(buffers_iterator_type::begin(cb),
                     buffers_iterator_type::end(cb),
                     out.begin(),
                     out.end()));
    mcpp::suffix_buffer_sequence sbs(cb,
                                     b.size() - 2);
    auto pair = zlib::inflate(inflate,
                              sbs,
                              std::move(inflated_buffer),
                              in.size(),
                              ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.first == boost::asio::buffer_size(sbs));
    CHECK(pair.second.size() == 300);
    auto decompressed = pair.second.data();
    CHECK(std::equal(buffers_iterator_type::begin(decompressed),
                     buffers_iterator_type::end(decompressed),
                     in.begin(),
                     in.end()));
  }
}

}
}
