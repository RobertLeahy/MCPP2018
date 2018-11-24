/**
 *  \file
 */

#pragma once

#include <boost/core/noncopyable.hpp>
#include <openssl/rsa.h>

namespace mcpp::crypto {

/**
 *  An RAII wrapper for an `RSA` object from
 *  OpenSSL.
 */
class rsa : private boost::noncopyable {
public:
  /**
   *  A type alias for `RSA*`.
   */
  using native_handle_type = ::RSA*;
  /**
   *  Calls `RSA_new` and assumes ownership
   *  of the result.
   */
  rsa();
  /**
   *  Assumes ownership of an `RSA*`.
   *
   *  \param [in] handle
   *    The `RSA*` to assume ownership of. The
   *    newly-constructed object will clean up
   *    the handle and the caller must not do so.
   *    Must not be `nullptr`.
   */
  explicit rsa(native_handle_type handle) noexcept;
  /**
   *  Assumes ownership of the handle managed by
   *  another object, leaving that object in an
   *  unspecified but valid state (i.e. it may be
   *  assigned to or destroyed but nothing else).
   *
   *  \param [in] other
   *    The object from which ownership shall be
   *    assumed.
   */
  rsa(rsa&& other) noexcept;
  /**
   *  Replaces the managed handle (destroying it if
   *  necessary) with the handle managed by another
   *  object, leaving that object in an unspecified
   *  but valid state (i.e. it may be assigned to or
   *  destroyed but nothing else).
   *
   *  \param [in] rhs
   *    The object from which ownership shall be
   *    assumed.
   *
   *  \return
   *    A reference to this object.
   */
  rsa& operator=(rsa&& rhs) noexcept;
  /**
   *  Calls `RSA_free` on the managed handle (if
   *  any).
   */
  ~rsa() noexcept;
  /**
   *  Retrieves the managed handle.
   *
   *  \return
   *    The `RSA*`.
   */
  native_handle_type native_handle() noexcept;
private:
  void destroy() noexcept;
  native_handle_type handle_;
};

}
