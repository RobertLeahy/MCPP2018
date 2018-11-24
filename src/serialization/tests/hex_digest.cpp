#include <mcpp/serialization/hex_digest.hpp>

#include <array>
#include <iterator>
#include <string>
#include <boost/asio/buffer.hpp>
#include <mcpp/crypto/evp_md_ctx.hpp>
#include <openssl/evp.h>

#include <catch2/catch.hpp>

namespace mcpp::serialization::tests {
namespace {

TEST_CASE("to_hex_digest",
          "[mcpp][serialization][hex_digest]")
{
  crypto::evp_md_ctx ctx;
  std::string material;
  int result = ::EVP_DigestInit_ex(ctx.native_handle(),
                                   ::EVP_sha1(),
                                   nullptr);
  REQUIRE(result);
  std::string out;
  std::array<unsigned char,
             EVP_MAX_MD_SIZE> digest;
  unsigned int i;
  SECTION("\"Notch\"") {
    material = "Notch";
    auto ec = crypto::evp_digest_update(ctx.native_handle(),
                                        boost::asio::buffer(material));
    REQUIRE_FALSE(ec);
    result = ::EVP_DigestFinal_ex(ctx.native_handle(),
                                  digest.data(),
                                  &i);
    REQUIRE(result);
    REQUIRE(i == 20);
    to_hex_digest(boost::asio::buffer(digest.data(),
                                      i),
                  std::back_inserter(out));
    std::string expected("4ed1f46bbe04bc756bcb17c0c7ce3e4632f06a48");
    CHECK(out == expected);
  }
  SECTION("\"jeb_\"") {
    material = "jeb_";
    auto ec = crypto::evp_digest_update(ctx.native_handle(),
                                        boost::asio::buffer(material));
    REQUIRE_FALSE(ec);
    result = ::EVP_DigestFinal_ex(ctx.native_handle(),
                                  digest.data(),
                                  &i);
    REQUIRE(result);
    REQUIRE(i == 20);
    to_hex_digest(boost::asio::buffer(digest.data(),
                                      i),
                  std::back_inserter(out));
    std::string expected("-7c9d5b0044c130109a5d7b5fb5c317c02b4e28c1");
    CHECK(out == expected);
  }
  SECTION("\"simon\"") {
    material = "simon";
    auto ec = crypto::evp_digest_update(ctx.native_handle(),
                                        boost::asio::buffer(material));
    REQUIRE_FALSE(ec);
    result = ::EVP_DigestFinal_ex(ctx.native_handle(),
                                  digest.data(),
                                  &i);
    REQUIRE(result);
    REQUIRE(i == 20);
    to_hex_digest(boost::asio::buffer(digest.data(),
                                      i),
                  std::back_inserter(out));
    std::string expected("88e16a1019277b15d58faf0541e11910eb756f6");
    CHECK(out == expected);
  }
  SECTION("Negative carry") {
    unsigned char arr[] = {0xff, 0xff, 0};
    to_hex_digest(boost::asio::buffer(arr),
                  std::back_inserter(out));
    std::string expected("-100");
    CHECK(out == expected);
  }
}

}
}
