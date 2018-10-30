/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <system_error>
#include <utility>
#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include "inflate_stream.hpp"
#include <mcpp/system_error.hpp>
#include "system_error.hpp"

namespace mcpp::zlib {

namespace detail {

enum class inflate_error {
  success = 0,
  max_size
};

std::error_code make_error_code(inflate_error) noexcept;

template<typename Allocator,
         typename MutableBufferSequence>
std::pair<boost::asio::const_buffer,
          std::size_t> inflate(basic_inflate_stream<Allocator>& stream,
                               boost::asio::const_buffer in,
                               MutableBufferSequence out,
                               std::error_code& ec)
{
  ec = stream.in(in);
  std::size_t retr = 0;
  if (ec) {
    return std::pair(in,
                     retr);
  }
  for (auto begin = boost::asio::buffer_sequence_begin(out), end = boost::asio::buffer_sequence_end(out);
       begin != end;
       ++begin)
  {
    boost::asio::mutable_buffer buffer(*begin);
    if (!buffer.size()) {
      continue;
    }
    ec = stream.out(buffer);
    if (ec) {
      break;
    }
    ec = zlib::make_error_code(::inflate(stream.native_handle(),
                                         Z_SYNC_FLUSH));
    std::size_t read = in.size() - stream.in().size();
    in += read;
    std::size_t written = buffer.size() - stream.out().size();
    retr += written;
    if (ec) {
      break;
    }
  }
  return std::pair(in,
                   retr);
}

template<typename Allocator,
         typename DynamicBuffer>
std::pair<boost::asio::const_buffer,
          DynamicBuffer> inflate(basic_inflate_stream<Allocator>& stream,
                                 boost::asio::const_buffer in,
                                 DynamicBuffer out,
                                 std::optional<std::size_t> hint,
                                 std::error_code& ec)
{
  ec.clear();
  std::optional<boost::asio::const_buffer> buffer(in);
  for (;;) {
    std::size_t alloc = std::numeric_limits<std::uint16_t>::max();
    if (hint) {
      alloc = *hint;
      hint = std::nullopt;
    }
    std::size_t rem = out.max_size() - out.size();
    if (!rem) {
      ec = make_error_code(inflate_error::max_size);
      return std::pair(in,
                       std::move(out));
    }
    alloc = std::min(alloc,
                     rem);
    auto mb = out.prepare(alloc);
    assert(buffer);
    auto pair = detail::inflate(stream,
                                *buffer,
                                mb,
                                ec);
    buffer.emplace(pair.first);
    out.commit(pair.second);
    if (ec) {
      break;
    }
    if (!buffer->size()) {
      break;
    }
  }
  assert(buffer);
  return std::pair(*buffer,
                   std::move(out));
}

}

/**
 *  Decompresses an entire compressed stream.
 *
 *  \tparam Allocator
 *    The `ProtoAllocator` associated with the instantiation
 *    of \ref basic_inflate_stream.
 *  \tparam ConstBufferSequence
 *    The type used to represent the input sequence.
 *  \tparam DynamicBuffer
 *    The `DynamicBuffer` used to allocate storage for the output
 *    sequence.
 *
 *  \param [in] stream
 *    The \ref basic_inflate_stream "inflate stream".
 *  \param [in] in
 *    The input sequence.
 *  \param [in] out
 *    The `DynamicBuffer` which shall be used to allocate space for
 *    and hold the output sequence. If decompression proceeds until
 *    such time that `out.size() == out.max_size()` then `ec` shall
 *    be set such that it has the same error condition as
 *    `std::errc::not_enough_memory`.
 *  \param [in] hint
 *    An optional hint about how to allocate storage. If the decompressed
 *    size of the stream indicated by `in` is known this value should be
 *    set to that known value, otherwise this is the block size which
 *    should be prepared by `out` on each demcompression iteration.
 *  \param [out] ec
 *    A `std::error_code` which shall receive the result of the
 *    operation.
 *
 *  \return
 *    A `std::pair` whose first element is the number of bytes consumed
 *    from `in` (in the success case this first element shall be exactly
 *    equal to `boost::asio::buffer_size(in)`) and whose second element
 *    is `out` after the appropriate calls to `prepare` and `commit` have
 *    been made so as to write the output sequence.
 */
template<typename Allocator,
         typename ConstBufferSequence,
         typename DynamicBuffer>
std::pair<std::size_t,
          DynamicBuffer> inflate(basic_inflate_stream<Allocator>& stream,
                                 ConstBufferSequence in,
                                 DynamicBuffer out,
                                 std::optional<std::size_t> hint,
                                 std::error_code& ec)
{
  stream.reset();
  ec.clear();
  std::optional<DynamicBuffer> buffer(std::move(out));
  std::size_t retr = 0;
  for (auto begin = boost::asio::buffer_sequence_begin(in), end = boost::asio::buffer_sequence_end(in);
       begin != end;
       ++begin)
  {
    assert(buffer);
    boost::asio::const_buffer cb(*begin);
    auto [remaining, dynamic] = detail::inflate(stream,
                                                cb,
                                                std::move(*buffer),
                                                hint,
                                                ec);
    hint = std::nullopt;
    assert(boost::asio::buffer_size(cb) >= boost::asio::buffer_size(remaining));
    retr += boost::asio::buffer_size(cb) - boost::asio::buffer_size(remaining);
    buffer.emplace(std::move(dynamic));
    if (ec) {
      if (ec == zlib::make_error_code(Z_BUF_ERROR)) {
        ec.clear();
        continue;
      }
      if (mcpp::is_eof(ec)) {
        ec.clear();
      }
      assert(buffer);
      return std::pair(retr,
                       std::move(*buffer));
    }
    assert(!remaining.size());
  }
  for (;;) {
    assert(buffer);
    auto before = buffer->size();
    auto pair = detail::inflate(stream,
                                boost::asio::const_buffer(),
                                std::move(*buffer),
                                hint,
                                ec);
    buffer.emplace(std::move(pair.second));
    if (ec) {
      if (ec == zlib::make_error_code(Z_BUF_ERROR)) {
        if (before == buffer->size()) {
          break;
        }
        continue;
      }
      if (mcpp::is_eof(ec)) {
        ec.clear();
        break;
      }
    }
  }
  assert(buffer);
  return std::pair(retr,
                   std::move(*buffer));
}

/**
 *  Decompresses an entire compressed stream.
 *
 *  \tparam Allocator
 *    The `ProtoAllocator` associated with the instantiation
 *    of \ref basic_inflate_stream.
 *  \tparam ConstBufferSequence
 *    The type used to represent the input sequence.
 *  \tparam DynamicBuffer
 *    The `DynamicBuffer` used to allocate storage for the output
 *    sequence.
 *
 *  \param [in] stream
 *    The \ref basic_inflate_stream "inflate stream".
 *  \param [in] in
 *    The input sequence.
 *  \param [in] out
 *    The `DynamicBuffer` which shall be used to allocate space for
 *    and hold the output sequence. If decompression proceeds until
 *    such time that `out.size() == out.max_size()` then `ec` shall
 *    be set such that it has the same error condition as
 *    `std::errc::not_enough_memory`.
 *  \param [out] ec
 *    A `std::error_code` which shall receive the result of the
 *    operation.
 *
 *  \return
 *    A `std::pair` whose first element is the number of bytes consumed
 *    from `in` (in the success case this first element shall be exactly
 *    equal to `boost::asio::buffer_size(in)`) and whose second element
 *    is `out` after the appropriate calls to `prepare` and `commit` have
 *    been made so as to write the output sequence.
 */
template<typename Allocator,
         typename ConstBufferSequence,
         typename DynamicBuffer>
std::pair<std::size_t,
          DynamicBuffer> inflate(basic_inflate_stream<Allocator>& stream,
                                 ConstBufferSequence in,
                                 DynamicBuffer out,
                                 std::error_code& ec)
{
  return zlib::inflate(stream,
                       in,
                       std::move(out),
                       std::nullopt,
                       ec);
}

}
