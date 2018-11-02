/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <system_error>
#include <type_traits>
#include <utility>
#include <boost/asio/error.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/system/error_code.hpp>
#include <mcpp/async_result.hpp>
#include <mcpp/completion_handler_base.hpp>
#include <mcpp/system_error.hpp>
#include "varint.hpp"

namespace mcpp::serialization {

namespace detail {

template<typename Integer,
         typename AsyncReadStream,
         typename DynamicBuffer,
         typename CompletionHandler>
class read_varint_completion_handler : public completion_handler_base<decltype(std::declval<AsyncReadStream&>().get_executor()),
                                                                      CompletionHandler>
{
private:
  using base = completion_handler_base<decltype(std::declval<AsyncReadStream&>().get_executor()),
                                       CompletionHandler>;
public:
  read_varint_completion_handler(AsyncReadStream& stream,
                                 DynamicBuffer&& buffer,
                                 CompletionHandler&& h)
    : base              (stream.get_executor(),
                         std::move(h)),
      stream_           (stream),
      buffer_           (std::move(buffer)),
      buffer_size_      (buffer_.size()),
      bytes_transferred_(0)
  {}
  void initiate() {
    assert(bytes_transferred_ < varint_max_size<Integer>);
    auto buffers = buffer_.prepare(1);
    stream_.async_read_some(buffers,
                            std::move(*this));
  }
  void operator()(boost::system::error_code ec,
                  std::size_t bytes_transferred)
  {
    bytes_transferred_ += bytes_transferred;
    assert(bytes_transferred_ <= varint_max_size<Integer>);
    buffer_.commit(bytes_transferred);
    if (ec) {
      invoke(ec,
             0);
      return;
    }
    assert(bytes_transferred);
    std::error_code sec;
    auto buffers = buffer_.data();
    using iterator_type = boost::asio::buffers_iterator<decltype(buffers),
                                                        std::byte>;
    auto begin = iterator_type::begin(buffers);
    assert(buffer_.size() > buffer_size_);
    begin += buffer_size_;
    auto end = iterator_type::end(buffers);
    auto pair = serialization::from_varint<Integer>(begin,
                                                    end,
                                                    sec);
    if (sec) {
      if (mcpp::is_eof(sec)) {
        initiate();
        return;
      }
      invoke(sec,
             0);
    }
    assert(pair.second == end);
    invoke(std::error_code(),
           pair.first);
  }
private:
  void invoke(boost::system::error_code ec,
              Integer i)
  {
    invoke(to_error_code(ec),
           i);
  }
  void invoke(std::error_code ec,
              Integer i)
  {
    if constexpr (std::is_invocable_v<CompletionHandler,
                                      std::error_code,
                                      std::size_t,
                                      Integer,
                                      DynamicBuffer>)
    {
      base::invoke(ec,
                   bytes_transferred_,
                   i,
                   std::move(buffer_));
    } else {
      base::invoke(ec,
                   bytes_transferred_,
                   i);
    }
  }
  AsyncReadStream& stream_;
  DynamicBuffer    buffer_;
  std::size_t      buffer_size_;
  std::size_t      bytes_transferred_;
};

}

/**
 *  Asynchronously reads a varint.
 *
 *  \tparam Integer
 *    The integer type.
 *  \tparam AsyncReadStream
 *    A model of `AsyncReadStream`.
 *  \tparam DynamicBuffer
 *    A model of `DynamicBuffer`.
 *  \tparam CompletionToken
 *    A completion token whose associated completion handler
 *    has signature:<br>
 *    `void(std::error_code, std::size_t, Integer);`<br>
 *    Where the parameters are: The result of the operation,
 *    the number of bytes transferred, and the object of type
 *    `Integer` which was read (assuming the operation succeeded).
 *    Note that if possible the completion handler will be called
 *    with the following signature:<br>
 *    `void(std::error_code, std::size_t, Integer, DynamicBuffer)`<br>
 *    Where the added final parameter provides a means for the
 *    `DynamicBuffer` to be returned. In this case the input sequence
 *    of the returned `DynamicBuffer` shall contain whatever bytes it
 *    originally contained plus whatever bytes were read by this
 *    operation.
 *
 *  \param [in] stream
 *    The `AsyncReadStream` from which to read.
 *  \param [in] buffer
 *    The `DynamicBuffer` into which data shall be read.
 *  \param [in] t
 *    The completion token which shall be used to complete the
 *    operation.
 *
 *  \return
 *    Whatever (if anything) is appropriate given `CompletionToken`
 *    and `t`.
 */
template<typename Integer,
         typename AsyncReadStream,
         typename DynamicBuffer,
         typename CompletionToken>
auto async_read_varint(AsyncReadStream& stream,
                       DynamicBuffer&& buffer,
                       CompletionToken&& t)
{
  using signature_type = void(std::error_code,
                              std::size_t,
                              Integer);
  using completion_handler_type = completion_handler_t<CompletionToken,
                                                       signature_type>;
  completion_handler_type h(std::forward<CompletionToken>(t));
  async_result_t<CompletionToken,
                 signature_type> result(h);
  using op_type = detail::read_varint_completion_handler<Integer,
                                                         AsyncReadStream,
                                                         std::decay_t<DynamicBuffer>,
                                                         completion_handler_type>;
  op_type(stream,
          std::forward<DynamicBuffer>(buffer),
          std::move(h)).initiate();
  return result.get();
}

}
