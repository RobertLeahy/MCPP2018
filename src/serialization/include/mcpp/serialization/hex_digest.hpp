/**
 *  \file
 */

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <system_error>
#include <utility>
#include <boost/asio/buffers_iterator.hpp>

namespace mcpp::serialization {

namespace detail {

template<typename OutputIterator>
std::pair<OutputIterator,
          bool> format_hex_digest_byte(std::uint8_t byte,
                                       bool first,
                                       OutputIterator out)
{
  if (first && !byte) {
    return std::pair(out,
                     true);
  }
  std::array<char,
             2> buffer;
  auto result = std::to_chars(buffer.data(),
                              buffer.data() + buffer.size(),
                              byte,
                              16);
  assert(result.ec == std::errc{});
  if (!first) {
    out = std::fill_n(out,
                      buffer.size() - std::size_t(result.ptr - buffer.data()),
                      '0');
  }
  return std::pair(std::copy(buffer.data(),
                             result.ptr,
                             out),
                   false);
}

template<typename BuffersIterator,
         typename OutputIterator>
OutputIterator to_negative_hex_digest(BuffersIterator begin,
                                      BuffersIterator end,
                                      OutputIterator out)
{
  *(out++) = '-';
  std::reverse_iterator rbegin(end);
  std::reverse_iterator rend(begin);
  auto carry_to = std::find_if_not(rbegin,
                                   rend,
                                   [](auto u) noexcept { return u == 0; }).base();
  //  If this were true it would imply
  //  that the entire range was zero, in
  //  which case the number isn't negative,
  //  and therefore this function shouldn't
  //  have been called
  assert(carry_to != begin);
  --carry_to;
  bool first = true;
  for (; begin != carry_to; ++begin) {
    std::uint8_t curr(*begin);
    curr = ~curr;
    auto pair = detail::format_hex_digest_byte(curr,
                                               first,
                                               out);
    out = pair.first;
    first = pair.second;
  }
  assert(begin == carry_to);
  for (; begin != end; ++begin) {
    std::uint8_t curr(*begin);
    curr = ~curr;
    ++curr;
    auto pair = detail::format_hex_digest_byte(curr,
                                               first,
                                               out);
    out = pair.first;
    first = pair.second;
  }
  return out;
}

template<typename BuffersIterator,
         typename OutputIterator>
OutputIterator to_positive_hex_digest(BuffersIterator begin,
                                      BuffersIterator end,
                                      OutputIterator out)
{
  bool first = true;
  for (; begin != end; ++begin) {
    auto pair = detail::format_hex_digest_byte(*begin,
                                               first,
                                               out);
    out = pair.first;
    first = pair.second;
  }
  return out;
}

}

/**
 *  Converts a buffer of bytes (nominally a SHA-1 digest)
 *  to a Minecraft-style hex digest string.
 *
 *  \tparam ConstBufferSequence
 *    A `ConstBufferSequence` type which shall be used
 *    to represent the input.
 *  \tparam OutputIterator
 *    An `OutputIterator` which shall be used to write a
 *    sequence of `char` objects constituting the output.
 *
 *  \param [in] cb
 *    The sequence of bytes to operate on.
 *  \param [in] out
 *    The output to write to.
 *
 *  \return
 *    `out` after being written to and incremented an
 *    appropriate number of times.
 */
template<typename ConstBufferSequence,
         typename OutputIterator>
OutputIterator to_hex_digest(ConstBufferSequence cb,
                             OutputIterator out)
{
  using buffers_iterator_type = boost::asio::buffers_iterator<ConstBufferSequence,
                                                              std::uint8_t>;
  auto begin = buffers_iterator_type::begin(cb);
  auto end = buffers_iterator_type::end(cb);
  if (begin == end) {
    return detail::to_positive_hex_digest(begin,
                                          end,
                                          out);
  }
  constexpr std::uint8_t max(std::numeric_limits<std::int8_t>::max());
  if (*begin > max) {
    return detail::to_negative_hex_digest(begin,
                                          end,
                                          out);
  }
  return detail::to_positive_hex_digest(begin,
                                        end,
                                        out);
}

}
