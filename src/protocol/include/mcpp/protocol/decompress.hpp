/**
 *  \file
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <system_error>
#include <utility>
#include <boost/asio/buffers_iterator.hpp>
#include <mcpp/serialization/varint.hpp>
#include <mcpp/suffix_buffer_sequence.hpp>
#include <mcpp/zlib/inflate.hpp>
#include <mcpp/zlib/inflate_stream.hpp>

namespace mcpp::protocol {

namespace detail {

enum class decompress_error {
  success = 0,
  length_negative,
  padded,
  wrong_length
};

std::error_code make_error_code(decompress_error) noexcept;

}

/**
 *  Decompresses a packet body according to the Minecraft
 *  protocol.
 *
 *  The size header which precedes all compressed bodies shall
 *  be read and passed to a predicate function which may return:
 *
 *  - `true` in which case decompression proceeds
 *  - `false` in which case the function ends without decompressing
 *    any bytes nor writing any bytes to the provided `DynamicBuffer`
 *    (note no error is reported in this case)
 *
 *  If the size header is found to be inaccurate (i.e. it does not
 *  match the decompressed body) an error is reported. Similarly if
 *  there are bytes after the compressed body an error shall be
 *  reported.
 *
 *  \tparam Allocator
 *    The `ProtoAllocator` associated with the
 *    \ref zlib::basic_inflate_stream "inflate stream" instantiation
 *    which will be used.
 *  \tparam ConstBufferSequence
 *    A `ConstBufferSequence` used to model the compressed body.
 *  \tparam DynamicBuffer
 *    A `DynamicBuffer` used to store the decompressed body (assuming
 *    anything is decompressed).
 *  \tparam ProceedPredicate
 *    The type of a predicate which is invocable with the following
 *    signature:
 *    \code
 *    bool(std::int32_t);
 *    \endcode
 *    Where the returned `bool` is used to decide whether to proceed
 *    with decompression or not, and the `std::int32_t` parameter is the
 *    size read from the provided buffer of bytes (this value is checked
 *    to ensure it is not negative before being passed through).
 *
 *  \param [in] stream
 *    The \ref zlib::basic_inflate_stream "inflate stream" which will
 *    be used for decompression.
 *  \param [in] in
 *    A buffer sequence containing the bytes of the body and compressed
 *    packet header.
 *  \param [in] out
 *    A `DynamicBuffer` which shall allocate memory to which the decompressed
 *    body shall be written (assuming anything is written).
 *  \param [in] pred
 *    The predicate which shall be used to decide whether to proceed with
 *    decompression.
 *  \param [out] ec
 *    A `std::error_code` which shall receive the result of
 *    the operation.
 *
 *  \return
 *    A `std::pair` of a `ConstBufferSequence` of all unconsumed bytes from
 *    `in` (if `pred` returns `true` and `ec` is falsey this is guaranteed to
 *    be an empty buffer) as the first element and `out` (after being written
 *    to) as the second element.
 */
template<typename Allocator,
         typename ConstBufferSequence,
         typename DynamicBuffer,
         typename ProceedPredicate>
std::pair<
#ifdef MCPP_DOXYGEN_RUNNING
unspecified_const_buffer_sequence
#else
mcpp::suffix_buffer_sequence<ConstBufferSequence>
#endif
,DynamicBuffer> decompress(zlib::basic_inflate_stream<Allocator>& stream,
                           ConstBufferSequence in,
                           DynamicBuffer out,
                           ProceedPredicate pred,
                           std::error_code& ec)
{
  ec.clear();
  using buffers_iterator_type = boost::asio::buffers_iterator<ConstBufferSequence,
                                                              std::byte>;
  auto end = buffers_iterator_type::end(in);
  auto pair = serialization::from_varint<std::int32_t>(buffers_iterator_type::begin(in),
                                                       end,
                                                       ec);
  std::size_t remaining(end - pair.second);
  mcpp::suffix_buffer_sequence pbs(in,
                                   remaining);
  if (ec) {
    return std::pair(pbs,
                     std::move(out));
  }
  if (pair.first < 0) {
    ec = make_error_code(detail::decompress_error::length_negative);
    return std::pair(pbs,
                     std::move(out));
  }
  if (!pred(pair.first)) {
    return std::pair(pbs,
                     std::move(out));
  }
  auto result = zlib::inflate(stream,
                              pbs,
                              std::move(out),
                              ec);
  assert(result.first <= remaining);
  mcpp::suffix_buffer_sequence after(in,
                                     remaining - result.first);
  std::pair retr(after,
                 std::move(result.second));
  if (ec) {
    return retr;
  }
  if (result.first != remaining) {
    ec = make_error_code(detail::decompress_error::padded);
    return retr;
  }
  if (result.second.size() != std::size_t(pair.first)) {
    ec = make_error_code(detail::decompress_error::wrong_length);
    return retr;
  }
  return retr;
}


}
