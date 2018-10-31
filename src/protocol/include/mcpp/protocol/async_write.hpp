/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <system_error>
#include <type_traits>
#include <utility>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/write.hpp>
#include <mcpp/async_result.hpp>
#include <mcpp/checked.hpp>
#include <mcpp/completion_handler_base.hpp>
#include <mcpp/serialization/async_write_varint.hpp>

namespace mcpp::protocol {

namespace detail {

template<typename AsyncWriteStream,
         typename DynamicBuffer,
         typename ConstBufferSequence,
         typename CompletionHandler>
class async_write_completion_handler : public completion_handler_base<decltype(std::declval<AsyncWriteStream&>().get_executor()),
                                                                      CompletionHandler>
{
private:
  using base = completion_handler_base<decltype(std::declval<AsyncWriteStream&>().get_executor()),
                                       CompletionHandler>;
public:
  async_write_completion_handler(AsyncWriteStream& stream,
                                 ConstBufferSequence cb,
                                 CompletionHandler&& h)
    : base              (stream.get_executor(),
                         std::move(h)),
      stream_           (stream),
      cb_               (std::move(cb)),
      bytes_transferred_(0)
  {}
  void operator()(std::error_code ec,
                  std::size_t bytes_transferred,
                  DynamicBuffer buffer)
  {
    assert(!buffer_);
    assert(!bytes_transferred_);
    bytes_transferred_ = bytes_transferred;
    buffer_.emplace(std::move(buffer));
    if (ec) {
      invoke(ec);
      return;
    }
    AsyncWriteStream& stream = stream_;
    ConstBufferSequence cb(cb_);
    boost::asio::async_write(stream,
                             cb,
                             std::move(*this));
  }
  void operator()(std::error_code ec,
                  std::size_t bytes_transferred)
  {
    assert(buffer_);
    assert(bytes_transferred_);
    bytes_transferred_ += bytes_transferred;
    invoke(ec);
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
  AsyncWriteStream&            stream_;
  std::optional<DynamicBuffer> buffer_;
  ConstBufferSequence          cb_;
  std::size_t                  bytes_transferred_;
};

}

/**
 *  Writes the Minecraft protocol envelope to an
 *  asynchronous stream.
 *
 *  \tparam AsyncWriteStream
 *    A model of `AsyncWriteStream`.
 *  \tparam DynamicBuffer
 *    A model of `DynamicBuffer`, used to acquire memory
 *    for the header.
 *  \tparam ConstBufferSequence
 *    A model of `ConstBufferSequence`, used to represent
 *    the memory of the body.
 *  \tparam CompletionToken
 *    A completion token whose associated completion handler
 *    has the following signature:
 *    \code
 *    void(std::error_code, //  Result of the operation
 *         std::size_t);    //  Number of bytes transferred
 *    \endcode
 *    Alternately the following signature (used preferentially)
 *    may be used:
 *    \code
 *    void(std::error_code,
 *         std::size_t,
 *         DynamicBuffer);  //  The provided DynamicBuffer after being used
 *    \endcode
 *
 *  \param [in] stream
 *    The stream to write to.
 *  \param [in] buffer
 *    The `DynamicBuffer` to use to obtain memory to hold the
 *    packet header.
 *  \param [in] cb
 *    A `ConstBufferSequence` containing the bytes of the body.
 *  \param [in] t
 *    The completion token.
 *
 *  \return
 *    Whatever is appropriate given `CompletionToken` and `t`.
 */
template<typename AsyncWriteStream,
         typename DynamicBuffer,
         typename ConstBufferSequence,
         typename CompletionToken>
auto async_write(AsyncWriteStream& stream,
                 DynamicBuffer&& buffer,
                 ConstBufferSequence&& cb,
                 CompletionToken&& t)
{
  using signature_type = void(std::error_code,
                              std::size_t);
  using completion_handler_type = completion_handler_t<CompletionToken,
                                                       signature_type>;
  completion_handler_type h(std::forward<CompletionToken>(t));
  async_result_t<CompletionToken,
                 signature_type> result(h);
  auto size = mcpp::checked_cast<std::uint32_t>(boost::asio::buffer_size(cb));
  if (!size) {
    auto ex = boost::asio::get_associated_executor(h,
                                                   stream.get_executor());
    auto bound = std::bind(std::move(h),
                           make_error_code(std::errc::value_too_large),
                           0);
    boost::asio::post(boost::asio::bind_executor(ex,
                                                 std::move(bound)));
    return result.get();
  }
  using op_type = detail::async_write_completion_handler<AsyncWriteStream,
                                                         std::decay_t<DynamicBuffer>,
                                                         std::decay_t<ConstBufferSequence>,
                                                         completion_handler_type>;
  op_type op(stream,
             std::forward<ConstBufferSequence>(cb),
             std::move(h));
  serialization::async_write_varint(stream,
                                    *size,
                                    std::forward<DynamicBuffer>(buffer),
                                    std::move(op));
  return result.get();
}

}
