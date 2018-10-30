#include <mcpp/zlib/deflate.hpp>

#include <algorithm>
#include <cstddef>
#include <system_error>
#include <utility>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <mcpp/zlib/deflate_stream.hpp>
#include <mcpp/zlib/inflate.hpp>
#include <mcpp/zlib/inflate_stream.hpp>

#include <catch2/catch.hpp>

namespace mcpp::zlib::tests {
namespace {

using vector_type = std::vector<std::byte>;
using dynamic_buffer_type = boost::asio::dynamic_vector_buffer<vector_type::value_type,
                                                               vector_type::allocator_type>;

TEST_CASE("deflate (single buffer)",
          "[mcpp][deflate][zlib]")
{
  vector_type out;
  dynamic_buffer_type buffer(out);
  vector_type in;
  std::error_code ec;
  deflate_stream stream;
  vector_type inflated;
  dynamic_buffer_type inflated_buffer(inflated);
  inflate_stream inflate;
  SECTION("Empty") {
    auto pair = deflate(stream,
                        boost::asio::buffer(in),
                        std::move(buffer),
                        ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.first == 0);
    CHECK(pair.second.size() != 0);
    auto inflate_pair = zlib::inflate(inflate,
                                      pair.second.data(),
                                      std::move(inflated_buffer),
                                      ec);
    REQUIRE_FALSE(ec);
    CHECK(inflate_pair.first == pair.second.size());
    CHECK(inflate_pair.second.size() == 0);
  }
  SECTION("Non-empty") {
    in.resize(300);
    unsigned char u(0);
    for (auto&& b : in) {
      b = std::byte{u++};
    }
    auto pair = deflate(stream,
                        boost::asio::buffer(in),
                        std::move(buffer),
                        ec);
    REQUIRE_FALSE(ec);
    CHECK(pair.first == in.size());
    CHECK(pair.second.size() != 0);
    auto inflate_pair = zlib::inflate(inflate,
                                      pair.second.data(),
                                      std::move(inflated_buffer),
                                      ec);
    REQUIRE_FALSE(ec);
    CHECK(inflate_pair.first == pair.second.size());
    auto cb = inflate_pair.second.data();
    using buffers_iterator_type = boost::asio::buffers_iterator<decltype(cb),
                                                                std::byte>;
    CHECK(std::equal(buffers_iterator_type::begin(cb),
                     buffers_iterator_type::end(cb),
                     in.begin(),
                     in.end()));
  }
  SECTION("DynamicBuffer with limited size") {
    in.resize(1000);
    dynamic_buffer_type buffer(out,
                               1);
    CHECK_THROWS(deflate(stream,
                         boost::asio::buffer(in),
                         std::move(buffer),
                         ec));
  }
}

}
}
