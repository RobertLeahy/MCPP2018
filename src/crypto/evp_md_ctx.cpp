#include <mcpp/crypto/evp_md_ctx.hpp>

#include <cassert>
#include <new>
#include <string>
#include <system_error>
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

namespace detail {

std::error_code make_error_code(evp_digest_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Crypto/EVP Message Digest";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<evp_digest_error>(code)) {
      case evp_digest_error::success:
        return "Success";
      case evp_digest_error::update_failed:
        return "EVP_DigestUpdate failed";
      default:
        break;
      }
      return "Unknown";
    }
  } category;
  return std::error_code(static_cast<int>(err),
                         category);
}

}

}
