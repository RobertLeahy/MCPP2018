/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <system_error>
#include <type_traits>
#include <utility>
#include <boost/asio/read.hpp>
#include <boost/system/error_code.hpp>
#include <mcpp/async_result.hpp>
#include <mcpp/completion_handler_base.hpp>
#include <mcpp/serialization/async_read_varint.hpp>
#include <mcpp/system_error.hpp>

namespace mcpp::protocol {

namespace detail {

template<typename AsyncReadStream,
         typename DynamicBuffer,
         typename AfterReadLengthInitiatingFunction,
         typename CompletionHandler>
class async_read_completion_handler : public completion_handler_base<decltype(std::declval<AsyncReadStream&>().get_executor()),
                                                                     CompletionHandler>
{
private:
  using base = completion_handler_base<decltype(std::declval<AsyncReadStream&>().get_executor()),
                                       CompletionHandler>;
public:
  async_read_completion_handler(AsyncReadStream& stream,
                                DynamicBuffer buffer,
                                AfterReadLengthInitiatingFunction after_read_length,
                                CompletionHandler h)
    : base              (stream.get_executor(),
                         std::move(h)),
      stream_           (stream),
      buffer_           (std::move(buffer)),
      after_read_length_(std::move(after_read_length)),
      bytes_transferred_(0),
      length_           (0)
  {}
  void initiate() {
    assert(buffer_);
    auto&& stream = stream_;
    auto buffer = get_buffer();
    serialization::async_read_varint<std::uint32_t>(stream,
                                                    std::move(buffer),
                                                    std::move(*this));
  }
  void operator()(std::error_code ec,
                  std::size_t bytes_transferred,
                  std::uint32_t length,
                  DynamicBuffer buffer)
  {
    assert(!buffer_);
    bytes_transferred_ += bytes_transferred;
    if (ec) {
      buffer_.emplace(std::move(buffer));
      invoke(ec);
      return;
    }
    length_ = length;
    auto after_read_length = std::move(after_read_length_);
    after_read_length(bytes_transferred,
                      length,
                      std::move(buffer),
                      std::move(*this));
  }
  void operator()(std::error_code ec,
                  DynamicBuffer buffer)
  {
    assert(!buffer_);
    buffer_.emplace(std::move(buffer));
    if (ec) {
      invoke(ec);
      return;
    }
    if (!length_) {
      invoke(std::error_code());
      return;
    }
    auto buffers = buffer_->prepare(length_);
    auto&& stream = stream_;
    boost::asio::async_read(stream,
                            std::move(buffers),
                            std::move(*this));
  }
  void operator()(boost::system::error_code ec,
                  std::size_t bytes_transferred)
  {
    assert(buffer_);
    bytes_transferred_ += bytes_transferred;
    buffer_->commit(bytes_transferred);
    invoke(to_error_code(ec));
  }
private:
  void invoke(std::error_code ec) {
    assert(buffer_);
    if constexpr (std::is_invocable_v<CompletionHandler,
                                      std::error_code,
                                      std::size_t,
                                      DynamicBuffer>)
    {
      base::invoke(ec,
                   bytes_transferred_,
                   std::move(*buffer_));
    } else {
      base::invoke(ec,
                   bytes_transferred_);
    }
  }
  DynamicBuffer get_buffer() {
    assert(buffer_);
    auto retr = std::move(*buffer_);
    buffer_.reset();
    return retr;
  }
  AsyncReadStream&                  stream_;
  std::optional<DynamicBuffer>      buffer_;
  AfterReadLengthInitiatingFunction after_read_length_;
  std::size_t                       bytes_transferred_;
  std::uint32_t                     length_;
};

}

/**
 *  Reads a single Minecraft protocol packet without parsing
 *  or decompressing it: Simply reads the length and then a
 *  number of bytes determined by that length.
 *
 *  \tparam AsyncReadStream
 *    A model of `AsyncReadStream`.
 *  \tparam DynamicBuffer
 *    A model of `DynamicBuffer`.
 *  \tparam AfterReadLengthInitiatingFunction
 *    An intermediate initiating function whose associated
 *    asynchronous operation will be initiated after the
 *    length is read. This initiating function should have the
 *    following signature (or equivalent):
 *    \code
 *    template<typename CompletionHandler>
 *    void(std::size_t,        // Number of bytes transferredd
 *         std::uint32_t,      // Length
 *         DynamicBuffer,      // Buffer in use for read operation
 *         CompletionHandler); // Completion handler for intermediate operation
 *    \endcode
 *    The initiated asynchronous operation is expected to invoke
 *    its completion handler with the following signature:
 *    \code
 *    void(std::error_code, // Result of operation
 *         DynamicBuffer);  // Buffer that should be used for remainder of read operation
 *    \endcode
 *    Note that unlike other initiating functions several of the
 *    requirements on initiating functions are relaxed for this
 *    initiating function:
 *    - It may invoke its completion handler from within the
 *      initiating function
 *    - It need not resolve completion tokens
 *  \tparam CompletionToken
 *    A completion token whose associated completion handler
 *    has the following signature:
 *    \code
 *    void(std::error_code, // Result of operation
 *         std::size_t);    // Bytes transferred
 *    \endcode
 *    Alternately the following signature is supported to allow
 *    seamless reuse of the `DynamicBuffer`:
 *    \code
 *    void(std::error_code,
 *         std::size_t,
 *         DynamicBuffer); // Buffer that was used for read operation
 *    \endcode
 *
 *  \param [in] stream
 *    The stream from which to read.
 *  \param [in] buffer
 *    The buffer into which to write.
 *  \param [in] after_read_length
 *    The intermediate initiating function which will be invoked
 *    after the packet's length is read.
 *  \param [in] t
 *    The completion token.
 *
 *  \return
 *    Whatever is appropriate given `CompletionToken` and `t`.
 */
template<typename AsyncReadStream,
         typename DynamicBuffer,
         typename AfterReadLengthInitiatingFunction,
         typename CompletionToken>
auto async_read(AsyncReadStream& stream,
                DynamicBuffer&& buffer,
                AfterReadLengthInitiatingFunction&& after_read_length,
                CompletionToken&& t)
{
  using signature_type = void(std::error_code,
                              std::size_t);
  using completion_handler_type = completion_handler_t<CompletionToken,
                                                       signature_type>;
  completion_handler_type h(std::forward<CompletionToken>(t));
  async_result_t<CompletionToken,
                 signature_type> result(h);
  using op_type = detail::async_read_completion_handler<AsyncReadStream,
                                                        std::decay_t<DynamicBuffer>,
                                                        std::decay_t<AfterReadLengthInitiatingFunction>,
                                                        completion_handler_type>;
  op_type(stream,
          std::forward<DynamicBuffer>(buffer),
          std::forward<AfterReadLengthInitiatingFunction>(after_read_length),
          std::move(h)).initiate();
  return result.get();
}

/**
 *  An `AfterReadLengthInitiatingFunction` for use with
 *  \ref async_read(AsyncReadStream&, DynamicBuffer&&, AfterReadLengthInitiatingFunction&&, CompletionToken&&) "async_read"
 *  which does nothing.
 */
class null_after_read_length_initiating_function {
public:
#ifndef MCPP_DOXYGEN_RUNNING
  template<typename DynamicBuffer,
           typename CompletionHandler>
  void operator()(std::size_t,
                  std::uint32_t,
                  DynamicBuffer buffer,
                  CompletionHandler handler)
  {
    handler(std::error_code(),
            std::move(buffer));
  }
#endif
};

/**
 *  An `AfterReadLengthInitiatingFunction` for use with
 *  \ref async_read(AsyncReadStream&, DynamicBuffer&&, AfterReadLengthInitiatingFunction&&, CompletionToken&&) "async_read"
 *  which consumes the bytes of the length from the
 *  `DynamicBuffer` before initiating another asynchronous
 *  operation.
 *
 *  \tparam Next
 *    The type of the intermediate initiating function which
 *    will be invoked once bytes have been consumed from the
 *    `DynamicBuffer`.
 */
template<typename Next>
class basic_consume_after_read_length_initiating_function {
public:
  /**
   *  Creates a basic_consume_after_read_length_initiating_function.
   *
   *  \param [in] next
   *    The intermediate initiating function which is next in the
   *    chain. Defaults to a default constructed `Next`.
   */
  explicit basic_consume_after_read_length_initiating_function(Next next = Next()) noexcept(std::is_nothrow_move_constructible_v<Next>)
    : next_(std::move(next))
  {}
#ifndef MCPP_DOXYGEN_RUNNING
  template<typename DynamicBuffer,
           typename CompletionHandler>
  void operator()(std::size_t bytes_transferred,
                  std::uint32_t length,
                  DynamicBuffer buffer,
                  CompletionHandler handler)
  {
    buffer.consume(bytes_transferred);
    next_(bytes_transferred,
          length,
          std::move(buffer),
          std::move(handler));
  }
#endif
private:
  Next next_;
};

/**
 *  A type alias for \ref basic_consume_after_read_length_initiating_function
 *  with a next intermediate initiating function which does nothing
 *  (i.e. intermediate initiating functions of this type consume the
 *  bytes associated with the length from the `DynamicBuffer` and do
 *  nothing else).
 */
using consume_after_read_length_initiating_function = basic_consume_after_read_length_initiating_function<null_after_read_length_initiating_function>;

namespace detail {

enum class limit_after_read_length_initiating_function_error {
  success = 0,
  too_long
};

std::error_code make_error_code(limit_after_read_length_initiating_function_error) noexcept;

}

/**
 *  An `AfterReadLengthInitiatingFunction` for use with
 *  \ref async_read(AsyncReadStream&, DynamicBuffer&&, AfterReadLengthInitiatingFunction&&, CompletionToken&&) "async_read"
 *  which limits the size of body which will be read.
 *
 *  \tparam Next
 *    The type of the intermediate initiating function which
 *    will be invoked once the body length has been checked.
 */
template<typename Next>
class basic_limit_after_read_length_initiating_function {
public:
  /**
   *  Creates a basic_limit_after_read_length_initiating_function.
   *
   *  \param [in] max
   *    The maximum number of bytes (inclusive) to be read as the
   *    body of a packet.
   *  \param [in] next
   *    The intermediate initiating function which is next in the
   *    chain. Defaults to a default constructed `Next`.
   */
  explicit basic_limit_after_read_length_initiating_function(std::uint32_t max,
                                                             Next next = Next()) noexcept(std::is_nothrow_move_constructible_v<Next>)
    : max_ (max),
      next_(std::move(next))
  {}
#ifndef MCPP_DOXYGEN_RUNNING
  template<typename DynamicBuffer,
           typename CompletionHandler>
  void operator()(std::size_t bytes_transferred,
                  std::uint32_t length,
                  DynamicBuffer buffer,
                  CompletionHandler handler)
  {
    if (length > max_) {
      handler(make_error_code(detail::limit_after_read_length_initiating_function_error::too_long),
              std::move(buffer));
      return;
    }
    next_(bytes_transferred,
          length,
          std::move(buffer),
          std::move(handler));
  }
#endif
private:
  std::uint32_t max_;
  Next          next_;
};

/**
 *  A type alias for \ref basic_limit_after_read_length_initiating_function
 *  with a next intermediate initiating function which does nothing
 *  (i.e. intermediate initiating functions of this type check the size
 *  of the body and do nothing else).
 */
using limit_after_read_length_initiating_function = basic_limit_after_read_length_initiating_function<null_after_read_length_initiating_function>;

/**
 *  Functions identically to \ref async_read(AsyncReadStream&, DynamicBuffer&&, AfterReadLengthInitiatingFunction&&, CompletionToken&&) "async_read"
 *  except the `DynamicBuffer` does not include the bytes of
 *  the length when the operation completes.
 *
 *  \tparam AsyncReadStream
 *    See \ref async_read(AsyncReadStream&, DynamicBuffer&&, AfterReadLengthInitiatingFunction&&, CompletionToken&&) "async_read".
 *  \tparam DynamicBuffer
 *    See \ref async_read(AsyncReadStream&, DynamicBuffer&&, AfterReadLengthInitiatingFunction&&, CompletionToken&&) "async_read".
 *  \tparam CompletionToken
 *    See \ref async_read(AsyncReadStream&, DynamicBuffer&&, AfterReadLengthInitiatingFunction&&, CompletionToken&&) "async_read".
 *
 *  \param [in] stream
 *    See \ref async_read(AsyncReadStream&, DynamicBuffer&&, AfterReadLengthInitiatingFunction&&, CompletionToken&&) "async_read".
 *  \param [in] buffer
 *    See \ref async_read(AsyncReadStream&, DynamicBuffer&&, AfterReadLengthInitiatingFunction&&, CompletionToken&&) "async_read".
 *  \param [in] t
 *    See \ref async_read(AsyncReadStream&, DynamicBuffer&&, AfterReadLengthInitiatingFunction&&, CompletionToken&&) "async_read".
 *
 *  \return
 *    See \ref async_read(AsyncReadStream&, DynamicBuffer&&, AfterReadLengthInitiatingFunction&&, CompletionToken&&) "async_read".
 */
template<typename AsyncReadStream,
         typename DynamicBuffer,
         typename CompletionToken>
auto async_read(AsyncReadStream& stream,
                DynamicBuffer&& buffer,
                CompletionToken&& t)
{
  return protocol::async_read(stream,
                              std::forward<DynamicBuffer>(buffer),
                              consume_after_read_length_initiating_function(),
                              std::forward<CompletionToken>(t));
}

}
