#include <mcpp/crypto/evp_cipher_ctx.hpp>

#include <cassert>
#include <new>
#include <openssl/evp.h>

namespace mcpp::crypto {

evp_cipher_ctx::evp_cipher_ctx() {
  ctx_ = ::EVP_CIPHER_CTX_new();
  if (!ctx_) {
    throw std::bad_alloc();
  }
}

evp_cipher_ctx::evp_cipher_ctx(evp_cipher_ctx&& other) noexcept
  : ctx_(other.ctx_)
{
  other.ctx_ = nullptr;
}

evp_cipher_ctx& evp_cipher_ctx::operator=(evp_cipher_ctx&& rhs) noexcept {
  assert(&rhs != this);
  destroy();
  ctx_ = rhs.ctx_;
  rhs.ctx_ = nullptr;
  return *this;
}

evp_cipher_ctx::~evp_cipher_ctx() noexcept {
  destroy();
}

evp_cipher_ctx::native_handle_type evp_cipher_ctx::native_handle() noexcept {
  return ctx_;
}

void evp_cipher_ctx::destroy() noexcept {
  if (ctx_) {
    ::EVP_CIPHER_CTX_free(ctx_);
  }
}

}
