#include <mcpp/crypto/evp_md_ctx.hpp>

#include <cassert>
#include <new>
#include <openssl/evp.h>
#include <openssl/opensslv.h>

namespace mcpp::crypto {

namespace detail {

evp_md_ctx_policy::native_handle_type evp_md_ctx_policy::create() {
  native_handle_type retr = ::
#if OPENSSL_VERSION_NUMBER >= 0x10100000
  EVP_MD_CTX_new
#else
  EVP_MD_CTX_create
#endif
  ();
  if (!retr) {
    throw std::bad_alloc();
  }
  return retr;
}

void evp_md_ctx_policy::destroy(native_handle_type handle) noexcept {
  assert(handle);
  ::
#if OPENSSL_VERSION_NUMBER >= 0x10100000
  EVP_MD_CTX_free
#else
  EVP_MD_CTX_destroy
#endif
  (handle);
}

}

}
