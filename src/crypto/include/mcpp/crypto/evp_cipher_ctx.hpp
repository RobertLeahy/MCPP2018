/**
 *  \file
 */

#include <boost/core/noncopyable.hpp>
#include <openssl/evp.h>

namespace mcpp::crypto {

/**
 *  An RAII wrapper for a `EVP_CIPHER_CTX` from
 *  libcrypto.
 */
class evp_cipher_ctx : private boost::noncopyable {
public:
  /**
   *  A type alias for `EVP_CIPHER_CTX*`.
   */
  using native_handle_type = ::EVP_CIPHER_CTX*;
  /**
   *  Calls `EVP_CIPHER_CTX_new`.
   */
  evp_cipher_ctx();
  /**
   *  Assumes ownership of the `EVP_CIPHER_CTX` owned
   *  by another handle.
   *
   *  \param [in] other
   *    The evp_cipher_ctx from which ownership shall
   *    be assumed. This object shall be left in a state
   *    wherein it shall be safe to assign to, and to
   *    allow to be destroyed, but nothing else.
   */
  evp_cipher_ctx(evp_cipher_ctx&& other) noexcept;
  /**
   *  Assumes ownership of the `EVP_CIPHER_CTX` owned
   *  by another handle, destroying the `EVP_CIPHER_CTX`
   *  owned by this handle if necessary.
   *
   *  \param [in] rhs
   *    The evp_cipher_ctx from which ownership shall
   *    be assumed. This object shall be left in a state
   *    wherein it shall be safe to assign to, and to
   *    allow to be destroyed, but nothing else.
   *
   *  \return
   *    A reference to this object.
   */
  evp_cipher_ctx& operator=(evp_cipher_ctx&& rhs) noexcept;
  /**
   *  Calls `EVP_CIPHER_CTX_free` unless ownership of
   *  the `EVP_CIPHER_CTX` has been assumed by some
   *  other handle.
   */
  ~evp_cipher_ctx() noexcept;
  /**
   *  Retrieves the `EVP_CIPHER_CTX*` managed by this
   *  object.
   *
   *  \return
   *    The managed handle.
   */
  native_handle_type native_handle() noexcept;
private:
  void destroy() noexcept;
  native_handle_type ctx_;
};

}
