/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <system_error>
#include <type_traits>
#include <utility>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/error_code.hpp>
#include <mcpp/async_result.hpp>
#include <mcpp/completion_handler_base.hpp>
#include <mcpp/system_error.hpp>
#include "varint.hpp"

namespace mcpp::serialization {

namespace detail {

template<typename Executor,
         typename DynamicBuffer,
         typename CompletionHandler>
class write_varint_completion_handler : public completion_handler_base<Executor,
                                                                       CompletionHandler>
{
private:
  using base = completion_handler_base<Executor,
                                       CompletionHandler>;
public:
  write_varint_completion_handler(const Executor& ex,
                                  DynamicBuffer&& buffer,
                                  CompletionHandler&& h)
    : base   (ex,
              std::move(h)),
      buffer_(std::move(buffer))
  {}
  void operator()(boost::system::error_code ec,
                  std::size_t bytes_transferred)
  {
    buffer_.commit(bytes_transferred);
    auto sec = mcpp::to_error_code(ec);
    if constexpr (std::is_invocable_v<CompletionHandler,
                                      std::error_code,
                                      std::size_t,
                                      DynamicBuffer>)
    {
      base::invoke(sec,
                   bytes_transferred,
                   std::move(buffer_));
    } else {
      base::invoke(sec,
                   bytes_transferred);
    }
  }
private:
  DynamicBuffer buffer_;
};

}

/**
 *  Asynchronously writes the varint representation of an
 *  integer.
 *
 *  \tparam AsyncWriteStream
 *    A model of `AsyncWriteStream`.
 *  \tparam Integer
 *    The type of the integer to write.
 *  \tparam DynamicBuffer
 *    A model of `DynamicBuffer`.
 *  \tparam CompletionToken
 *    A completion token whose associated completion handler
 *    has the following signature:
 *    \code
 *    void(std::error_code, //  Result of operation
 *         std::size_t);    //  Number of bytes written
 *    \endcode
 *    For the sake of convenience and to enable reuse of the
 *    provided `DynamicBuffer` completion handlers with the
 *    following signature are also supported (and will be used
 *    preferentially when available):
 *    \code
 *    void(std::error_code,
 *         std::size_t,
 *         DynamicBuffer);  //  Buffer with all written bytes flushed to the input sequence
 *    \endcode
 *
 *  \param [in] stream
 *    The `AsyncWriteStream` to which the varint representation of
 *    `i` will be written.
 *  \param [in] i
 *    The integer whose representation shall be written.
 *  \param [in] buffer
 *    The `DynamicBuffer` from which storage for the representation of
 *    `i` shall be acquired.
 *  \param [in] t
 *    The completion token.
 *
 *  \return
 *    Whatever is appropriate given `CompletionToken` and `t`.
 */
template<typename AsyncWriteStream,
         typename Integer,
         typename DynamicBuffer,
         typename CompletionToken>
auto async_write_varint(AsyncWriteStream& stream,
                        Integer i,
                        DynamicBuffer&& buffer,
                        CompletionToken&& t)
{
  using signature_type = void(std::error_code,
                              std::size_t);
  using completion_handler_type = completion_handler_t<CompletionToken,
                                                       signature_type>;
  completion_handler_type h(std::forward<CompletionToken>(t));
  async_result_t<CompletionToken,
                 signature_type> result(h);
  auto bytes = serialization::varint_size(i);
  std::decay_t<DynamicBuffer> db(std::forward<DynamicBuffer>(buffer));
  auto mb = db.prepare(bytes);
  using buffers_iterator_type = boost::asio::buffers_iterator<decltype(mb),
                                                              std::byte>;
  auto iter = serialization::to_varint(i,
                                       buffers_iterator_type::begin(mb));
  assert(iter == buffers_iterator_type::end(mb));
  (void)iter;
  detail::write_varint_completion_handler op(stream.get_executor(),
                                             std::move(db),
                                             std::move(h));
  boost::asio::async_write(stream,
                           std::move(mb),
                           std::move(op));
  return result.get();
}

}
