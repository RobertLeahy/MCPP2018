/**
 *  \file
 */

#pragma once

#include <system_error>
#include <boost/asio/buffer.hpp>
#include <boost/core/noncopyable.hpp>
#include <openssl/evp.h>
#include "system_error.hpp"

namespace mcpp::crypto {

/**
 *  An RAII wrapper for an `EVP_MD_CTX`.
 */
class evp_md_ctx : private boost::noncopyable {
public:
  /**
   *  A type alias for `EVP_MD_CTX*`.
   */
  using native_handle_type = ::EVP_MD_CTX*;
  /**
   *  Creates an `EVP_MD_CTX` by calling
   *  `EVP_MD_CTX_new`.
   */
  evp_md_ctx();
  /**
   *  Assumes ownership of the `EVP_MD_CTX`
   *  owned by another object.
   *
   *  \param [in] other
   *    The object ownership of whose `EVP_MD_CTX`
   *    shall be assumed. After this constructor completes
   *    successfully it shall be safe to allow this
   *    object's lifetime to end or assign to it, the
   *    behavior of all other operations is undefined.
   */
  evp_md_ctx(evp_md_ctx&& other) noexcept;
  /**
   *  Replaces the owned `EVP_MD_CTX` (if any)
   *  with the `EVP_MD_CTX` owned by another
   *  object.
   *
   *  \param [in] rhs
   *    The object ownership of whose `EVP_MD_CTX`
   *    shall be assumed. After this function completes
   *    successfully it shall be safe to allow this
   *    object's lifetime to end or assign to it, the
   *    behavior of all other operations is undefined.
   *
   *  \return
   *    A reference to this object.
   */
  evp_md_ctx& operator=(evp_md_ctx&& rhs) noexcept;
  /**
   *  Calls `EVP_MD_CTX_free` if necessary.
   */
  ~evp_md_ctx() noexcept;
  /**
   *  Retrieves the owned `EVP_MD_CTX`.
   *
   *  \return
   *    A pointer to the owned `EVP_MD_CTX`.
   */
  native_handle_type native_handle() noexcept;
private:
  void destroy() noexcept;
  native_handle_type ctx_;
};

/**
 *  Calls `EVP_UpdateDigest` some number of times as
 *  necessary. The number of times `EVP_UpdateDigest`
 *  is called is equal to the number of `boost::asio::const_buffer`
 *  objects in the provided buffer sequence.
 *
 *  \tparam ConstBufferSequence
 *    A model of `ConstBufferSequence` which will be
 *    used to provide bytes to the message digest
 *    algorithm.
 *
 *  \param [in] handle
 *    The `EVP_MD_CTX*` to pass through to `EVP_UpdateDigest`
 *    call(s).
 *  \param [in] cb
 *    The sequence of buffers whose bytes shall be provided
 *    to the message digest algorithm.
 *
 *  \return
 *    A `std::error_code` representing the result of the
 *    operation.
 */
template<typename ConstBufferSequence>
std::error_code evp_digest_update(evp_md_ctx::native_handle_type handle,
                                  ConstBufferSequence cb)
{
  for (auto begin = boost::asio::buffer_sequence_begin(cb), end = boost::asio::buffer_sequence_end(cb);
       begin != end;
       ++begin)
  {
    boost::asio::const_buffer buffer(*begin);
    int result = ::EVP_DigestUpdate(handle,
                                    buffer.data(),
                                    buffer.size());
    if (!result) {
      return crypto::get_error_code();
    }
  }
  return std::error_code();
}

}
