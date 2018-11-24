#include <mcpp/crypto/evp_md_ctx.hpp>

#include <cassert>
#include <new>
#include <openssl/evp.h>
#include <openssl/opensslv.h>

namespace mcpp::crypto {

static evp_md_ctx::native_handle_type evp_md_ctx_new() noexcept {
  return ::
#if OPENSSL_VERSION_NUMBER >= 0x10100000
  EVP_MD_CTX_new
#else
  EVP_MD_CTX_create
#endif
  ();
}

static void evp_md_ctx_free(evp_md_ctx::native_handle_type handle) noexcept {
  ::
#if OPENSSL_VERSION_NUMBER >= 0x10100000
  EVP_MD_CTX_free
#else
  EVP_MD_CTX_destroy
#endif
  (handle);
}

evp_md_ctx::evp_md_ctx()
  : ctx_(evp_md_ctx_new())
{
  if (!ctx_) {
    throw std::bad_alloc();
  }
}

evp_md_ctx::evp_md_ctx(evp_md_ctx&& other) noexcept
  : ctx_(other.ctx_)
{
  other.ctx_ = nullptr;
}

evp_md_ctx& evp_md_ctx::operator=(evp_md_ctx&& rhs) noexcept {
  assert(this != &rhs);
  destroy();
  ctx_ = rhs.ctx_;
  rhs.ctx_ = nullptr;
  return *this;
}

evp_md_ctx::~evp_md_ctx() noexcept {
  destroy();
}

evp_md_ctx::native_handle_type evp_md_ctx::native_handle() noexcept {
  assert(ctx_);
  return ctx_;
}

void evp_md_ctx::destroy() noexcept {
  if (ctx_) {
    evp_md_ctx_free(ctx_);
  }
}

}
