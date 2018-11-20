#include <mcpp/crypto/evp_cipher_stream.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <mcpp/crypto/evp_cipher_ctx.hpp>
#include <mcpp/test/buffer_async_read_stream.hpp>
#include <mcpp/test/buffer_async_write_stream.hpp>
#include <mcpp/test/handler.hpp>
#include <openssl/evp.h>

#include <catch2/catch.hpp>

namespace mcpp::crypto::tests {
namespace {

TEST_CASE("evp_cipher_stream::async_write_some",
          "[mcpp][crypto][evp][async]")
{
  using vector_type = std::vector<std::byte>;
  using dynamic_buffer_type = boost::asio::dynamic_vector_buffer<vector_type::value_type,
                                                                 vector_type::allocator_type>;
  vector_type vec;
  test::write_handler_state state;
  boost::asio::io_context ctx;
  test::buffer_async_write_stream write_stream(ctx.get_executor());
  evp_cipher_stream stream(write_stream,
                           evp_cipher_ctx(),
                           dynamic_buffer_type(vec));
  const unsigned char key[] = {0, 1, 2, 3, 4, 5, 6, 7,
                               8, 9, 10, 11, 12, 13, 14, 15};
  static_assert(sizeof(key) == (128 / 8));
  const unsigned char iv[] = {16, 17, 18, 19, 20, 21, 22, 23,
                              24, 25, 26, 27, 28, 29, 20, 31};
  static_assert(sizeof(iv) == (128 / 8));
  int result = ::EVP_CipherInit_ex(stream.cipher_ctx().native_handle(),
                                   ::EVP_aes_128_cfb(),
                                   nullptr,
                                   key,
                                   iv,
                                   true);
  REQUIRE(result);
  static_assert(std::is_same_v<decltype(stream),
                               evp_cipher_stream<decltype(write_stream)&,
                                                 dynamic_buffer_type>>);
  SECTION("Empty") {
    stream.async_write_some(boost::asio::const_buffer(),
                            test::write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ctx.poll();
    CHECK(handlers != 0);
    CHECK(state.invoked);
    CHECK_FALSE(state.ec);
    CHECK(state.bytes_transferred == 0);
    CHECK(write_stream.written() == 0);
  }
  SECTION("Non-empty") {
    std::byte buffer[16];
    write_stream.buffer(buffer,
                        sizeof(buffer));
    std::vector<std::byte> in;
    in.push_back(std::byte{0});
    in.push_back(std::byte{1});
    in.push_back(std::byte{2});
    in.push_back(std::byte{3});
    in.push_back(std::byte{4});
    stream.async_write_some(boost::asio::buffer(in),
                            test::write_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ctx.poll();
    CHECK(handlers != 0);
    CHECK(state.invoked);
    CHECK_FALSE(state.ec);
    CHECK(state.bytes_transferred == in.size());
    CHECK(write_stream.written() == in.size());
    evp_cipher_ctx ctx;
    result = ::EVP_CipherInit_ex(ctx.native_handle(),
                                 ::EVP_aes_128_cfb(),
                                 nullptr,
                                 key,
                                 iv,
                                 false);
    REQUIRE(result);
    std::byte decrypted[5];
    auto ec = evp_cipher_update(ctx.native_handle(),
                                boost::asio::const_buffer(buffer,
                                                          write_stream.written()),
                                boost::asio::buffer(decrypted));
    REQUIRE_FALSE(ec);
    using std::begin;
    using std::end;
    CHECK(std::equal(begin(decrypted),
                     end(decrypted),
                     in.begin(),
                     in.end()));
  }
}

TEST_CASE("evp_cipher_stream::async_read_some",
          "[mcpp][crypto][evp][async]")
{
  using vector_type = std::vector<std::byte>;
  using dynamic_buffer_type = boost::asio::dynamic_vector_buffer<vector_type::value_type,
                                                                 vector_type::allocator_type>;
  vector_type vec;
  test::read_handler_state state;
  boost::asio::io_context ctx;
  test::buffer_async_read_stream read_stream(ctx.get_executor());
  evp_cipher_stream stream(read_stream,
                           evp_cipher_ctx(),
                           dynamic_buffer_type(vec));
  const unsigned char key[] = {0, 1, 2, 3, 4, 5, 6, 7,
                               8, 9, 10, 11, 12, 13, 14, 15};
  static_assert(sizeof(key) == (128 / 8));
  const unsigned char iv[] = {16, 17, 18, 19, 20, 21, 22, 23,
                              24, 25, 26, 27, 28, 29, 20, 31};
  static_assert(sizeof(iv) == (128 / 8));
  int result = ::EVP_CipherInit_ex(stream.cipher_ctx().native_handle(),
                                   ::EVP_aes_128_cfb(),
                                   nullptr,
                                   key,
                                   iv,
                                   true);
  REQUIRE(result);
  static_assert(std::is_same_v<decltype(stream),
                               evp_cipher_stream<decltype(read_stream)&,
                                                 dynamic_buffer_type>>);
  SECTION("Empty") {
    stream.async_read_some(boost::asio::mutable_buffer(),
                           test::read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ctx.poll();
    CHECK(handlers != 0);
    CHECK(state.invoked);
    CHECK_FALSE(state.ec);
    CHECK(state.bytes_transferred == 0);
    CHECK(read_stream.read() == 0);
  }
  SECTION("Non-empty (read exactly all)") {
    std::vector<std::byte> in;
    in.push_back(std::byte{0});
    in.push_back(std::byte{1});
    in.push_back(std::byte{2});
    in.push_back(std::byte{3});
    in.push_back(std::byte{4});
    read_stream.buffer(in.data(),
                       in.size());
    std::byte buffer[5];
    stream.async_read_some(boost::asio::buffer(buffer),
                           test::read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ctx.poll();
    CHECK(handlers != 0);
    CHECK(state.invoked);
    CHECK_FALSE(state.ec);
    CHECK(state.bytes_transferred == 5);
    REQUIRE(read_stream.read() == 5);
    evp_cipher_ctx ctx;
    result = ::EVP_CipherInit_ex(ctx.native_handle(),
                                 ::EVP_aes_128_cfb(),
                                 nullptr,
                                 key,
                                 iv,
                                 false);
    REQUIRE(result);
    std::byte decrypted[5];
    auto ec = evp_cipher_update(ctx.native_handle(),
                                boost::asio::buffer(buffer),
                                boost::asio::buffer(decrypted));
    REQUIRE_FALSE(ec);
    using std::begin;
    using std::end;
    CHECK(std::equal(begin(decrypted),
                     end(decrypted),
                     in.begin(),
                     in.end()));
  }
  SECTION("Non-empty (read too much)") {
    std::vector<std::byte> in;
    in.push_back(std::byte{0});
    in.push_back(std::byte{1});
    in.push_back(std::byte{2});
    in.push_back(std::byte{3});
    in.push_back(std::byte{4});
    read_stream.buffer(in.data(),
                       in.size());
    std::byte buffer[6];
    stream.async_read_some(boost::asio::buffer(buffer),
                           test::read_handler(state));
    CHECK_FALSE(state.invoked);
    auto handlers = ctx.poll();
    CHECK(handlers != 0);
    CHECK(state.invoked);
    CHECK_FALSE(state.ec);
    CHECK(state.bytes_transferred == 5);
    REQUIRE(read_stream.read() == 5);
    evp_cipher_ctx ctx;
    result = ::EVP_CipherInit_ex(ctx.native_handle(),
                                 ::EVP_aes_128_cfb(),
                                 nullptr,
                                 key,
                                 iv,
                                 false);
    REQUIRE(result);
    std::byte decrypted[5];
    auto ec = evp_cipher_update(ctx.native_handle(),
                                boost::asio::const_buffer(buffer,
                                                          5),
                                boost::asio::buffer(decrypted));
    REQUIRE_FALSE(ec);
    using std::begin;
    using std::end;
    CHECK(std::equal(begin(decrypted),
                     end(decrypted),
                     in.begin(),
                     in.end()));
  }
}

}
}
