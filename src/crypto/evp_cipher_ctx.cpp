#include <mcpp/crypto/evp_cipher_ctx.hpp>

#include <algorithm>
#include <cassert>
#include <new>
#include <string>
#include <system_error>
#include <utility>
#include <mcpp/checked.hpp>
#include <mcpp/crypto/system_error.hpp>
#include <openssl/evp.h>

namespace mcpp::crypto {

namespace detail {

evp_cipher_ctx_policy::native_handle_type evp_cipher_ctx_policy::create() {
  native_handle_type retr = ::EVP_CIPHER_CTX_new();
  if (!retr) {
    throw std::bad_alloc();
  }
  return retr;
}

void evp_cipher_ctx_policy::destroy(native_handle_type handle) noexcept {
  assert(handle);
  ::EVP_CIPHER_CTX_free(handle);
}

std::error_code make_error_code(evp_cipher_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Crypto/EVP Cipher";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<evp_cipher_error>(code)) {
      case evp_cipher_error::success:
        return "Success";
      case evp_cipher_error::overflow:
        return "Overflow converting buffer size";
      case evp_cipher_error::out_overflow:
        return "Overflow converting output size";
      case evp_cipher_error::in_overflow:
        return "Overflow converting input size";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<evp_cipher_error>(code)) {
      case evp_cipher_error::success:
        return std::error_condition();
      case evp_cipher_error::overflow:
      case evp_cipher_error::out_overflow:
      case evp_cipher_error::in_overflow:
        return make_error_code(std::errc::value_too_large).default_error_condition();
      default:
        break;
      }
      return std::error_condition(code,
                                  *this);
    }
  } category;
  return std::error_code(static_cast<int>(err),
                         category);
}

std::pair<boost::asio::const_buffer,
          boost::asio::mutable_buffer> evp_cipher_update(evp_cipher_ctx::native_handle_type handle,
                                                         boost::asio::const_buffer cb,
                                                         boost::asio::mutable_buffer mb,
                                                         std::error_code& ec)
{
  assert(::EVP_CIPHER_CTX_block_size(handle) == 1);
  ec.clear();
  std::pair retr(cb,
                 mb);
  auto to_process = std::min(retr.first.size(),
                             retr.second.size());
  auto inl = mcpp::checked_cast<int>(to_process);
  if (!inl) {
    ec = make_error_code(evp_cipher_error::overflow);
    return retr;
  }
  auto outl = *inl;
  int result = ::EVP_CipherUpdate(handle,
                                  reinterpret_cast<unsigned char*>(retr.second.data()),
                                  &outl,
                                  reinterpret_cast<const unsigned char*>(retr.first.data()),
                                  *inl);
  if (!result) {
    ec = crypto::get_error_code();
    return retr;
  }
  assert(outl == *inl);
  retr.first += to_process;
  retr.second += to_process;
  return retr;
}

}

}
