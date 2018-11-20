/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <system_error>
#include <type_traits>
#include <utility>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/write.hpp>
#include <boost/core/noncopyable.hpp>
#include "evp_cipher_ctx.hpp"
#include <mcpp/async_result.hpp>
#include <mcpp/completion_handler_base.hpp>
#include <mcpp/lowest_layer.hpp>
#include <mcpp/prefix_buffer_sequence.hpp>
#include <mcpp/remove_rvalue_reference.hpp>
#include <mcpp/system_error.hpp>

namespace mcpp::crypto {

/**
 *  Layers on top of an `AsyncReadStream` or
 *  `AsyncWriteStream` (or something which models
 *  both simultaneously) and transparently encrypts
 *  or decrypts the data which is written and/or
 *  read.
 *
 *  Note that objects of this type manage a single
 *  \ref evp_cipher_ctx and therefore encrypt or
 *  decrypt but not both, nor do they do one in one
 *  direction and the opposite in the other.
 *
 *  \tparam Stream
 *    An `AsyncReadStream` or `AsyncWriteStream` (or
 *    a type which models both simultaneously) to which
 *    plaintext or ciphertext (depending on the mode of
 *    the EVP context) shall be read and/or written.
 *  \tparam DynamicBuffer
 *    A `DynamicBuffer` which shall be used to obtain
 *    the buffer sequences needed to support reading or
 *    writing operations (since an intermediate encryption
 *    or decryption operation must occur).
 */
template<typename Stream,
         typename DynamicBuffer>
class evp_cipher_stream {
public:
  /**
   *  A type alias for the `Stream` template paremeter
   *  with all ref-qualification stripped.
   */
  using next_layer_type = std::remove_reference_t<Stream>;
  /**
   *  A type alias for `next_layer_type::lowest_layer_type`
   *  if such a type exists, `next_layer_type` otherwise.
   */
  using lowest_layer_type = lowest_layer_t<next_layer_type>;
  /**
   *  A type alias for the `Executor` type associated with
   *  `Stream`.
   */
  using executor_type = decltype(std::declval<Stream>().get_executor());
private:
  class state : private boost::noncopyable {
  public:
    template<typename T>
    state(T&& t,
          evp_cipher_ctx ctx,
          DynamicBuffer buffer)
      : next_layer(std::forward<T>(t)),
        ctx       (std::move(ctx)),
        buffer    (std::move(buffer))
    {}
    Stream         next_layer;
    evp_cipher_ctx ctx;
    DynamicBuffer  buffer;
  };
  using state_pointer_type = std::shared_ptr<state>;
  template<typename MutableBufferSequence,
           typename CompletionHandler>
  class read_op : public completion_handler_base<executor_type,
                                                 CompletionHandler>
  {
  private:
    using base = completion_handler_base<executor_type,
                                         CompletionHandler>;
    using destination_buffers_type = typename DynamicBuffer::mutable_buffers_type;
  public:
    read_op(MutableBufferSequence mb,
            destination_buffers_type dest,
            CompletionHandler h,
            state_pointer_type state)
      : base  (state->next_layer.get_executor(),
               std::move(h)),
        mb_   (mb),
        dest_ (dest),
        state_(std::move(state))
    {}
    template<typename ErrorCode>
    void operator()(ErrorCode ec,
                    std::size_t bytes_transferred)
    {
      assert(state_);
      if (bytes_transferred) {
        prefix_buffer_sequence dest_prefix(dest_,
                                           bytes_transferred);
        prefix_buffer_sequence mb_prefix(mb_,
                                         bytes_transferred);
        auto ec = crypto::evp_cipher_update(state_->ctx.native_handle(),
                                            dest_prefix,
                                            mb_prefix);
        if (ec) {
          base::get()(ec,
                      0);
          return;
        }
      }
      base::get()(mcpp::to_error_code(ec),
                  bytes_transferred);
    }
  private:
    MutableBufferSequence    mb_;
    destination_buffers_type dest_;
    state_pointer_type       state_;
  };
public:
  /**
   *  Creates an evp_cipher_stream.
   *
   *  \tparam T
   *    The type to forward through to a constructor
   *    of the underlying stream.
   *
   *  \param [in] t
   *    An object to forward through to a constructor
   *    of the underlying stream.
   *  \param [in] ctx
   *    The \ref evp_cipher_ctx which shall be used
   *    for encryption or decryption operations.
   *  \param [in] buffer
   *    The `DynamicBuffer` which shall be used to
   *    obtain intermediate memory buffers for
   *    encryption or decryption operations.
   */
  template<typename T>
  evp_cipher_stream(T&& t,
                    evp_cipher_ctx ctx,
                    DynamicBuffer buffer)
    : state_(std::make_shared<state>(std::forward<T>(t),
                                     std::move(ctx),
                                     std::move(buffer)))
  {}
  /**
   *  @{
   *  Obtains the next layer: The underlying stream
   *  object.
   *
   *  \return
   *    A reference to the next layer.
   */
  next_layer_type& next_layer() noexcept {
    assert(state_);
    return state_->next_layer;
  }
  const next_layer_type& next_layer() const noexcept {
    assert(state_);
    return state_->next_layer;
  }
  /**
   *  @}
   *  @{
   *  Obtains the lowest layer: The result of calling
   *  `lowest_layer()` on the underlying stream if the
   *  underlying stream has a `lowest_layer_type` nested
   *  type alias, otherwise returns a reference to the
   *  underlying stream.
   *
   *  \return
   *    The lowest layer.
   */
  lowest_layer_type& lowest_layer() noexcept {
    assert(state_);
    return mcpp::get_lowest_layer(state_->next_layer);
  }
  const lowest_layer_type& lowest_layer() const noexcept {
    assert(state_);
    return mcpp::get_lowest_layer(state_->next_layer);
  }
  /**
   *  @}
   *  Returns the result of calling `get_executor` on
   *  the underlying stream.
   *
   *  \return
   *    The `Executor` associated with the underlying
   *    stream.
   */
  executor_type get_executor() noexcept {
    assert(state_);
    return state_->next_layer.get_executor();
  }
  /**
   *  @{
   *  Obtains a reference to the \ref evp_cipher_ctx.
   *
   *  \return
   *    A reference to the \ref evp_cipher_ctx.
   */
  evp_cipher_ctx& cipher_ctx() noexcept {
    assert(state_);
    return state_->ctx;
  }
  const evp_cipher_ctx& cipher_ctx() const noexcept {
    assert(state_);
    return state_->ctx;
  }
  /**
   *  @}
   *  Asynchronously writes data to the underlying
   *  stream after encrypting or decrypting it. Which
   *  operation is performed depends on the settings
   *  of the \ref evp_cipher_ctx "cipher context".
   *
   *  \tparam ConstBufferSequence
   *    A `ConstBufferSequence` type from which plaintext
   *    or ciphertext shall be drawn.
   *  \tparam CompletionToken
   *    A completion token whose associated completion
   *    handler type has the following signature:
   *    \code
   *    void(std::error_code, // Result of the operation
   *         std::size_t);    // Number of bytes written
   *    \endcode
   *
   *  \param [in] cb
   *    The sequence of buffers to write.
   *  \param [in] t
   *    The completion token.
   *
   *  \return
   *    Whatever is appropriate given `CompletionToken` and
   *    `t`.
   */
  template<typename ConstBufferSequence,
           typename CompletionToken>
  decltype(auto) async_write_some(ConstBufferSequence cb,
                                  CompletionToken&& t)
  {
    assert(state_);
    state_->buffer.consume(state_->buffer.size());
    assert(!state_->buffer.size());
    using signature_type = void(std::error_code,
                                std::size_t);
    using completion_handler_type = completion_handler_t<CompletionToken,
                                                         signature_type>;
    completion_handler_type h(std::forward<CompletionToken>(t));
    async_result_t<CompletionToken,
                   signature_type> result(h);
    auto bytes = boost::asio::buffer_size(cb);
    auto mb = state_->buffer.prepare(bytes);
    auto ec = crypto::evp_cipher_update(state_->ctx.native_handle(),
                                        cb,
                                        mb);
    if (ec) {
      auto ex = boost::asio::get_associated_executor(h,
                                                     get_executor());
      auto bound = std::bind(std::move(h),
                             std::error_code(),
                             0);
      auto bound_ex = boost::asio::bind_executor(ex,
                                                 std::move(bound));
      boost::asio::post(std::move(bound_ex));
      return result.get();
    }
    state_->buffer.commit(bytes);
    boost::asio::async_write(state_->next_layer,
                             state_->buffer.data(),
                             std::move(h));
    return result.get();
  }
  /**
   *  Asynchronously reads data from the underlying
   *  stream and then encrypts or decrypts it. Which
   *  operation is performed depends on the settings
   *  of the \ref evp_cipher_ctx "cipher context".
   *
   *  \tparam MutableBufferSequence
   *    A `MutableBufferSequence` to which ciphertext
   *    or plaintext shall be written.
   *  \tparam CompletionToken
   *    A completion token whose associated completion
   *    handler type has the following signature:
   *    \code
   *    void(std::error_code, // Result of the operation
   *         std::size_t);    // Number of bytes read
   *    \endcode
   *
   *  \param [in] mb
   *    The sequence of buffers which shall be written to.
   *  \param [in] t
   *    The completion token.
   *
   *  \return
   *    Whatever is appropriate given `CompletionToken` and
   *    `t`.
   */
  template<typename MutableBufferSequence,
           typename CompletionToken>
  decltype(auto) async_read_some(MutableBufferSequence mb,
                                 CompletionToken&& t)
  {
    assert(state_);
    state_->buffer.consume(state_->buffer.size());
    assert(!state_->buffer.size());
    using signature_type = void(std::error_code,
                                std::size_t);
    using completion_handler_type = completion_handler_t<CompletionToken,
                                                         signature_type>;
    completion_handler_type h(std::forward<CompletionToken>(t));
    async_result_t<CompletionToken,
                   signature_type> result(h);
    auto read_buffers = state_->buffer.prepare(boost::asio::buffer_size(mb));
    read_op op(mb,
               read_buffers,
               std::move(h),
               state_);
    state_->next_layer.async_read_some(read_buffers,
                                       std::move(op));
    return result.get();
  }
private:
  state_pointer_type state_;
};

template<typename T,
         typename U>
evp_cipher_stream(T&&,
                  evp_cipher_ctx,
                  U) -> evp_cipher_stream<remove_rvalue_reference_t<T&&>,
                                          U>;

}
