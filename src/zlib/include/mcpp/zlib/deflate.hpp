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
#include "deflate_stream.hpp"
#include <mcpp/checked.hpp>
#include "system_error.hpp"
#include <zlib.h>

namespace mcpp::zlib {

namespace detail {

enum class deflate_error {
  success = 0,
  bound_overflow
};

std::error_code make_error_code(deflate_error) noexcept;

template<typename Allocator,
         typename MutableBufferSequence>
std::pair<std::size_t,
          std::size_t> deflate(basic_deflate_stream<Allocator>& stream,
                               boost::asio::const_buffer in,
                               MutableBufferSequence out,
                               bool finish,
                               std::error_code& ec)
{
  std::pair retr(std::size_t(0),
                 std::size_t(0));
  ec = stream.in(in);
  if (ec) {
    return retr;
  }
  for (auto begin = boost::asio::buffer_sequence_begin(out), end = boost::asio::buffer_sequence_end(out);
       begin != end;
       ++begin)
  {
    boost::asio::mutable_buffer buffer(*begin);
    if (!buffer.size()) {
      continue;
    }
    ec = stream.out(out);
    if (ec) {
      return retr;
    }
    std::size_t in_before = stream.in().size();
    if (!in_before && !finish) {
      return retr;
    }
    std::size_t out_before = stream.out().size();
    int result = ::deflate(stream.native_handle(),
                           finish ? Z_FINISH : Z_NO_FLUSH);
    std::size_t in_after = stream.in().size();
    assert(in_before >= in_after);
    retr.first += in_before - in_after;
    std::size_t out_after = stream.out().size();
    assert(out_before >= out_after);
    retr.second += out_before - out_after;
    if (result == Z_OK) {
      continue;
    }
    if (result == Z_STREAM_END) {
      assert(finish);
      return retr;
    }
    ec = zlib::make_error_code(result);
    return retr;
  }
  assert(!finish);
  return retr;
}

}

/**
 *  Compresses a buffer of bytes.
 *
 *  \tparam Allocator
 *    The `ProtoAllocator` associated with the instantiation of
 *    \ref basic_deflate_stream being used.
 *  \tparam ConstBufferSequence
 *    The type used to represent the input sequence.
 *  \tparam DynamicBuffer
 *    The type used to represent the output sequence.
 *
 *  \param [in] stream
 *    The \ref basic_deflate_stream "stream" to use for compression.
 *    `deflateReset` will be called before any operations are performed.
 *  \param [in] in
 *    A `ConstBufferSequence` representing the input sequence.
 *  \param [in] out
 *    A `DynamicBuffer` to which all output bytes shall be written.
 *  \param [out] ec
 *    A `std::error_code` which shall receive the result of the operation.
 *
 *  \return
 *    A `std::pair` whose first element is the number of bytes from `in`
 *    actually compressed (if `ec` is falsey after the invocation this
 *    is guaranteed to be `0`) and whose second parameter is `out` after
 *    all bytes have been written thereto.
 */
template<typename Allocator,
         typename ConstBufferSequence,
         typename DynamicBuffer>
std::pair<std::size_t,
          DynamicBuffer> deflate(basic_deflate_stream<Allocator>& stream,
                                 ConstBufferSequence in,
                                 DynamicBuffer out,
                                 std::error_code& ec)
{
  stream.reset();
  ec.clear();
  std::pair retr(std::size_t(0),
                 std::move(out));
  std::size_t in_size = boost::asio::buffer_size(in);
  auto z_size = mcpp::checked_cast<::uLong>(in_size);
  if (!z_size) {
    ec = make_error_code(detail::deflate_error::bound_overflow);
    return retr;
  }
  auto bound = mcpp::checked_cast<std::size_t>(::deflateBound(stream.native_handle(),
                                                              *z_size));
  if (!bound) {
    ec = make_error_code(detail::deflate_error::bound_overflow);
    return retr;
  }
  assert(*bound);
  for (auto begin = boost::asio::buffer_sequence_begin(in), end = boost::asio::buffer_sequence_end(in);
       begin != end;
       ++begin)
  {
    assert(*bound);
    auto mb = retr.second.prepare(*bound);
    boost::asio::const_buffer buffer(*begin);
    auto pair = detail::deflate(stream,
                                buffer,
                                mb,
                                false,
                                ec);
    retr.first += pair.first;
    retr.second.commit(pair.second);
    *bound -= pair.second;
    if (ec) {
      return retr;
    }
  }
  assert(*bound);
  auto mb = retr.second.prepare(*bound);
  auto pair = detail::deflate(stream,
                              boost::asio::const_buffer(),
                              mb,
                              true,
                              ec);
  retr.first += pair.first;
  retr.second.commit(pair.second);
  return retr;
}

}
