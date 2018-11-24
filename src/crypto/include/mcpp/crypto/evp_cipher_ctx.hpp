/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <optional>
#include <system_error>
#include <utility>
#include <boost/asio/buffer.hpp>
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

namespace detail {

enum class evp_cipher_error {
  success = 0,
  overflow,
  out_overflow,
  in_overflow
};

std::error_code make_error_code(evp_cipher_error) noexcept;

std::pair<boost::asio::const_buffer,
          boost::asio::mutable_buffer> evp_cipher_update(evp_cipher_ctx::native_handle_type,
                                                         boost::asio::const_buffer,
                                                         boost::asio::mutable_buffer,
                                                         std::error_code&);

}

/**
 *  Calls `EVP_CipherUpdate` one or more times to decrypt
 *  or encrypt the contents of a buffer sequence into another
 *  buffer sequence.
 *
 *  It must be the case that `boost::asio::buffer_size(cb) == boost::asio::buffer_size(mb)`
 *  or the behavior is undefined.
 *
 *  \tparam ConstBufferSequence
 *    The `ConstBufferSequence` type from which to read.
 *  \tparam MutableBufferSequence
 *    The `MutableBufferSequence` type to which to write.
 *
 *  \param [in] handle
 *    The \ref evp_cipher_ctx::native_handle_type "EVP handle"
 *    to use for the operation.
 *  \param [in] cb
 *    The `ConstBufferSequence` from which to read.
 *  \param [in] mb
 *    The `MutableBufferSequence` to which to write.
 *
 *  \return
 *    A `std::error_code` containing the result of the
 *    operation.
 */
template<typename ConstBufferSequence,
         typename MutableBufferSequence>
std::error_code evp_cipher_update(evp_cipher_ctx::native_handle_type handle,
                                  ConstBufferSequence cb,
                                  MutableBufferSequence mb)
{
  assert(boost::asio::buffer_size(cb) == boost::asio::buffer_size(mb));
  auto cb_begin = boost::asio::buffer_sequence_begin(cb);
  auto cb_end = boost::asio::buffer_sequence_end(cb);
  auto mb_begin = boost::asio::buffer_sequence_begin(mb);
  auto mb_end = boost::asio::buffer_sequence_end(mb);
  std::optional<boost::asio::const_buffer> in;
  std::optional<boost::asio::mutable_buffer> out;
  while ((cb_begin != cb_end) && (mb_begin != mb_end)) {
    if (!in) {
      in.emplace(*cb_begin);
    }
    if (!out) {
      out.emplace(*mb_begin);
    }
    assert(in);
    assert(out);
    std::error_code ec;
    auto pair = detail::evp_cipher_update(handle,
                                          *in,
                                          *out,
                                          ec);
    if (ec) {
      return ec;
    }
    in = pair.first;
    out = pair.second;
    assert(in);
    assert(out);
    if (!in->size()) {
      in = std::nullopt;
      ++cb_begin;
    }
    if (!out->size()) {
      out = std::nullopt;
      ++mb_begin;
    }
  }
  assert(!in);
  assert(!out);
  return std::error_code();
}

/**
 *  Calls `EVP_CipherUpdate` one or more times to decrypt
 *  or encrypt the contents of a buffer sequence.
 *
 *  \tparam ConstBufferSequence
 *    The `ConstBufferSequence` type from which to read.
 *  \tparam DynamicBuffer
 *    The `DynamicBuffer` type which shall be used to obtain
 *    memory into which to write.
 *
 *  \param [in] handle
 *    The \ref evp_cipher_ctx::native_handle_type "EVP handle"
 *    to use for the operation.
 *  \param [in] cb
 *    The `ConstBufferSequence` from which to read.
 *  \param [in] buffer
 *    The `DynamicBuffer` which shall be used to obtain memory
 *    into which to write.
 *  \param [out] ec
 *    A `std::error_code` which shall receive the result of the
 *    operation.
 *
 *  \return
 *    `buffer` after being written to.
 */
template<typename ConstBufferSequence,
         typename DynamicBuffer>
DynamicBuffer evp_cipher_update(evp_cipher_ctx::native_handle_type handle,
                                ConstBufferSequence cb,
                                DynamicBuffer buffer,
                                std::error_code& ec)
{
  ec.clear();
  auto bytes = boost::asio::buffer_size(cb);
  auto mb = buffer.prepare(bytes);
  ec = crypto::evp_cipher_update(handle,
                                 cb,
                                 mb);
  if (!ec) {
    buffer.commit(bytes);
  }
  return std::move(buffer);
}

}
