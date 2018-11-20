#include <mcpp/crypto/evp_cipher_ctx.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <system_error>
#include <utility>
#include <vector>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <openssl/evp.h>

#include <catch2/catch.hpp>

namespace mcpp::crypto::tests {
namespace {

TEST_CASE("evp_cipher_update (MutableBufferSequence)",
          "[mcpp][crypto][evp][evp_cipher_ctx]")
{
  evp_cipher_ctx ctx;
  const unsigned char key[] = {0, 1, 2, 3, 4, 5, 6, 7,
                               8, 9, 10, 11, 12, 13, 14, 15};
  static_assert(sizeof(key) == (128 / 8));
  const unsigned char iv[] = {16, 17, 18, 19, 20, 21, 22, 23,
                              24, 25, 26, 27, 28, 29, 20, 31};
  static_assert(sizeof(iv) == (128 / 8));
  int result = ::EVP_CipherInit_ex(ctx.native_handle(),
                                   ::EVP_aes_128_cfb(),
                                   nullptr,
                                   key,
                                   iv,
                                   true);
  REQUIRE(result);
  SECTION("Empty") {
    std::error_code ec = evp_cipher_update(ctx.native_handle(),
                                           boost::asio::const_buffer(),
                                           boost::asio::mutable_buffer());
    REQUIRE_FALSE(ec);
  }
  SECTION("Non-empty") {
    const unsigned char in[] = {0, 1, 2, 3, 4, 5, 6, 7,
                                8, 9, 10, 11, 12, 13, 14, 15};
    unsigned char out[16];
    static_assert(sizeof(in) == sizeof(out));
    std::error_code ec = evp_cipher_update(ctx.native_handle(),
                                           boost::asio::buffer(in),
                                           boost::asio::buffer(out));
    REQUIRE_FALSE(ec);
    unsigned char decrypted[16];
    evp_cipher_ctx ctx2;
    result = ::EVP_CipherInit_ex(ctx2.native_handle(),
                                 ::EVP_aes_128_cfb(),
                                 nullptr,
                                 key,
                                 iv,
                                 false);
    REQUIRE(result);
    ec = evp_cipher_update(ctx2.native_handle(),
                           boost::asio::buffer(out),
                           boost::asio::buffer(decrypted));
    REQUIRE_FALSE(ec);
    using std::begin;
    using std::end;
    CHECK(std::equal(begin(in),
                     end(in),
                     begin(decrypted),
                     end(decrypted)));
  }
  using vector_type = std::vector<std::byte>;
  vector_type in;
  vector_type out;
  using dynamic_buffer_type = boost::asio::dynamic_vector_buffer<vector_type::value_type,
                                                                 vector_type::allocator_type>;
  dynamic_buffer_type buffer(out);
  std::error_code ec;
  SECTION("DynamicBuffer empty") {
    auto d = evp_cipher_update(ctx.native_handle(),
                               boost::asio::buffer(in),
                               std::move(buffer),
                               ec);
    REQUIRE_FALSE(ec);
    CHECK(out.empty());
    CHECK(d.data().size() == 0);
  }
  SECTION("DynamicBuffer non-empty") {
    in.push_back(std::byte{0});
    in.push_back(std::byte{1});
    in.push_back(std::byte{2});
    in.push_back(std::byte{3});
    in.push_back(std::byte{4});
    auto d = evp_cipher_update(ctx.native_handle(),
                               boost::asio::buffer(in),
                               std::move(buffer),
                               ec);
    REQUIRE_FALSE(ec);
    CHECK(out.size() == 5);
    CHECK(d.data().size() == 5);
    evp_cipher_ctx ctx2;
    result = ::EVP_CipherInit_ex(ctx2.native_handle(),
                                 ::EVP_aes_128_cfb(),
                                 nullptr,
                                 key,
                                 iv,
                                 false);
    REQUIRE(result);
    vector_type decrypted;
    dynamic_buffer_type decrypted_buffer(decrypted);
    auto d2 = evp_cipher_update(ctx2.native_handle(),
                                d.data(),
                                std::move(decrypted_buffer),
                                ec);
    REQUIRE_FALSE(ec);
    CHECK(std::equal(decrypted.begin(),
                     decrypted.end(),
                     in.begin(),
                     in.end()));
    auto data = d2.data();
    using buffers_iterator_type = boost::asio::buffers_iterator<decltype(data),
                                                                std::byte>;
    CHECK(std::equal(buffers_iterator_type::begin(data),
                     buffers_iterator_type::end(data),
                     in.begin(),
                     in.end()));
  }
}

}
}
