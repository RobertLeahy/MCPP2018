#include <mcpp/crypto/evp_md_ctx.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <string>
#include <boost/asio/buffer.hpp>
#include <openssl/evp.h>

#include <catch2/catch.hpp>

namespace mcpp::crypto::tests {
namespace {

TEST_CASE("evp_digest_update",
          "[mcpp][crypto][evp][evp_md_ctx][md]")
{
  evp_md_ctx ctx;
  std::string material;
  int result = ::EVP_DigestInit_ex(ctx.native_handle(),
                                   ::EVP_sha1(),
                                   nullptr);
  REQUIRE(result);
  std::array<unsigned char,
             EVP_MAX_MD_SIZE> out;
  unsigned int i;
  SECTION("Empty string") {
    auto ec = evp_digest_update(ctx.native_handle(),
                                boost::asio::buffer(material));
    REQUIRE_FALSE(ec);
    result = ::EVP_DigestFinal_ex(ctx.native_handle(),
                                  out.data(),
                                  &i);
    REQUIRE(result);
    REQUIRE(i == 20);
    unsigned char expected[] = {0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55, 0xbf, 0xef, 0x95, 0x60, 0x18, 0x90, 0xaf, 0xd8, 0x07, 0x09};
    using std::begin;
    using std::end;
    CHECK(std::equal(begin(expected),
                     end(expected),
                     out.data(),
                     out.data() + i));
  }
  SECTION("\"The quick brown fox jumps over the lazy dog\"") {
    material = "The quick brown fox jumps over the lazy dog";
    auto ec = evp_digest_update(ctx.native_handle(),
                                boost::asio::buffer(material));
    REQUIRE_FALSE(ec);
    result = ::EVP_DigestFinal_ex(ctx.native_handle(),
                                  out.data(),
                                  &i);
    REQUIRE(result);
    REQUIRE(i == 20);
    unsigned char expected[] = {0x2f, 0xd4, 0xe1, 0xc6, 0x7a, 0x2d, 0x28, 0xfc, 0xed, 0x84, 0x9e, 0xe1, 0xbb, 0x76, 0xe7, 0x39, 0x1b, 0x93, 0xeb, 0x12};
    using std::begin;
    using std::end;
    CHECK(std::equal(begin(expected),
                     end(expected),
                     out.data(),
                     out.data() + i));
  }
}

}
}
