/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <system_error>
#include <utility>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <mcpp/checked.hpp>
#include <mcpp/serialization/varint.hpp>
#include <mcpp/zlib/deflate.hpp>
#include <mcpp/zlib/deflate_stream.hpp>

namespace mcpp::protocol {

namespace detail {

enum class compress_error {
  success = 0,
  size_overflow
};

std::error_code make_error_code(compress_error) noexcept;

}

/**
 *  Compresses a packet body according to the Minecraft
 *  protocol.
 *
 *  Note that the output is not a wire format packet but
 *  is rather the inner body of a wire format packet as
 *  required when compression is enabled (i.e. there is
 *  no length header, this is expected to be added by
 *  \ref async_write).
 *
 *  \tparam Allocator
 *    The `ProtoAllocator` associated with the
 *    \ref zlib::basic_deflate_stream "deflate stream"
 *    instantiation which will be used.
 *  \tparam ConstBufferSequence
 *    A `ConstBufferSequence` used to model the body of
 *    the packet.
 *  \tparam DynamicBuffer
 *    A `DynamicBuffer` to use to store the compressed
 *    body.
 *
 *  \param [in] stream
 *    The \ref zlib::basic_deflate_stream "deflate stream"
 *    which will be used for compression.
 *  \param [in] in
 *    A buffer sequence containing the bytes of the
 *    uncompressed body.
 *  \param [in] out
 *    A `DynamicBuffer` which shall allocate memory to which
 *    the compressed body shall be written.
 *  \param [out] ec
 *    A `std::error_code` which shall receive the result of
 *    the operation.
 *
 *  \return
 *    `out` after the compressed body has been written to it.
 */
template<typename Allocator,
         typename ConstBufferSequence,
         typename DynamicBuffer>
DynamicBuffer compress(zlib::basic_deflate_stream<Allocator>& stream,
                       ConstBufferSequence in,
                       DynamicBuffer out,
                       std::error_code& ec)
{
  ec.clear();
  auto size = mcpp::checked_cast<std::int32_t>(boost::asio::buffer_size(in));
  if (!size) {
    ec = make_error_code(detail::compress_error::size_overflow);
    return std::move(out);
  }
  auto header_size = serialization::varint_size(*size);
  auto mb = out.prepare(header_size);
  using buffers_iterator_type = boost::asio::buffers_iterator<decltype(mb),
                                                              std::byte>;
  auto begin = buffers_iterator_type::begin(mb);
  auto end = serialization::to_varint(*size,
                                      begin);
  (void)end;
  assert(end == buffers_iterator_type::end(mb));
  out.commit(header_size);
  auto pair = zlib::deflate(stream,
                            in,
                            std::move(out),
                            ec);
  assert(ec ||
         (pair.first == boost::asio::buffer_size(in)));
  return std::move(pair.second);
}

}
