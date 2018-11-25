/**
 *  \file
 */

#pragma once

#include <system_error>
#include <boost/asio/buffer.hpp>
#include <mcpp/handle.hpp>
#include <openssl/evp.h>
#include "system_error.hpp"

namespace mcpp::crypto {

namespace detail {

class evp_md_ctx_policy : public pointer_handle_policy_base<::EVP_MD_CTX*> {
public:
  static native_handle_type create();
  static void destroy(native_handle_type) noexcept;
};

}

/**
 *  An instantiation of the \ref mcpp::handle "handle class template"
 *  which manages an `EVP_CIPHER_CTX*`.
 */
using evp_md_ctx = handle<detail::evp_md_ctx_policy>;

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
