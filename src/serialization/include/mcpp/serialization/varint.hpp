/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <system_error>
#include <type_traits>
#include <utility>
#include <mcpp/system_error.hpp>

namespace mcpp::serialization {

/**
 *  Yields the maximum number of bytes occupied
 *  by the varint representation of a certain integer
 *  type.
 *
 *  \tparam Integer
 *    The integer type.
 */
template<typename Integer>
constexpr std::size_t varint_max_size = ((sizeof(Integer) * 8) / 7) +
                                        (((sizeof(Integer) * 8) % 7) ? 1 : 0);

/**
 *  Determines the number of bytes required to represent
 *  a certain integer as a varint.
 *
 *  Note that since this function computes the size of the
 *  varint representation, not the ZigZag representation, the
 *  type is significant for negative values.
 *
 *  \tparam Integer
 *    The integer type.
 *
 *  \param [in] i
 *    The integer.
 *
 *  \return
 *    The number of bytes. Strictly positive with an upper
 *    bound of `varint_max_size<Integer>`.
 */
template<typename Integer>
constexpr std::size_t varint_size(Integer i) noexcept {
  std::make_unsigned_t<Integer> u(i);
  if (!u) {
    return 1;
  }
  std::size_t retr = 0;
  do {
    u >>= 7;
    ++retr;
  } while (u);
  return retr;
}

/**
 *  Obtains the varint representation of an integer.
 *
 *  \tparam Integer
 *    The integer type.
 *  \tparam OutputIterator
 *    A type which models `OutputIterator`.
 *
 *  \param [in] i
 *    The integer.
 *  \param [in] begin
 *    The iterator through which `std::byte` objects
 *    representing `i` shall be written.
 *
 *  \return
 *    The result of incrementing `begin` a number of
 *    times equal to the number of `std::byte` objects
 *    written.
 */
template<typename Integer,
         typename OutputIterator>
OutputIterator to_varint(Integer i,
                         OutputIterator begin)
{
  std::make_unsigned_t<Integer> u(i);
  if (!u) {
    *begin = std::byte{0};
    ++begin;
    return begin;
  }
  for (;;) {
    std::uint8_t mask(0b01111111);
    std::uint8_t curr(u & mask);
    u >>= 7;
    if (u) {
      std::uint8_t mask(0b10000000);
      curr |= mask;
      *begin = std::byte{curr};
      ++begin;
      continue;
    }
    *begin = std::byte{curr};
    ++begin;
    break;
  }
  return begin;
}

/**
 *  Obtains the ZigZag varint representation of a signed
 *  integer.
 *
 *  \tparam Integer
 *    The signed integer type.
 *  \tparam OutputIterator
 *    A type which models `OutputIterator`.
 *
 *  \param [in] i
 *    The signed integer.
 *  \param [in] begin
 *    The iterator through which `std::byte` objects
 *    representing `i` shall be written.
 *
 *  \return
 *    The result of incrementing `begin` a number of
 *    times equal to the number of `std::byte` objects
 *    written.
 */
template<typename Integer,
         typename OutputIterator>
OutputIterator to_zig_zag_varint(Integer i,
                                 OutputIterator begin)
{
  static_assert(std::is_signed_v<Integer>);
  if (i >= 0) {
    std::make_unsigned_t<Integer> u(i);
    u *= 2;
    return serialization::to_varint(u,
                                    begin);
  }
  ++i;
  i *= -1;
  std::make_unsigned_t<Integer> u(i);
  u *= 2;
  ++u;
  return serialization::to_varint(u,
                                  begin);
}

namespace detail {

enum class varint_error {
  success = 0,
  max,
  overlong,
  overflow,
  eof
};

std::error_code make_error_code(varint_error) noexcept;

}

/**
 *  Attempts to obtain an integer from a varint representation
 *  stored in a range of `std::byte` objects.
 *
 *  \tparam Integer
 *    The type of integer.
 *  \tparam InputIterator
 *    A type which models `InputIterator` and which may be dereferenced
 *    to obtain a `std::byte` value.
 *
 *  \param [in] begin
 *    An iterator to the first `std::byte` to consume.
 *  \param [in] end
 *    An iterator to one past the last `std::byte` to consume.
 *  \param [out] ec
 *    A `std::error_code` object which shall receive the result
 *    of the parse.
 *
 *  \return
 *    A `std::pair` containing the parsed integer as the first element,
 *    and an iterator to one past the last `std::byte` consumed as the
 *    second element. If `ec` is truthy after this function returns the
 *    the first element must be disregarded and the second element contains
 *    an iterator to the `std::byte` which provoked the error.
 */
template<typename Integer,
         typename InputIterator>
std::pair<Integer,
          InputIterator> from_varint(InputIterator begin,
                                     InputIterator end,
                                     std::error_code& ec)
{
  ec.clear();
  if (begin == end) {
    ec = make_error_code(detail::varint_error::eof);
    return std::pair(Integer(0),
                     begin);
  }
  std::make_unsigned_t<Integer> u(0);
  for (std::size_t i = 0; i < varint_max_size<Integer>; ++i) {
    assert(begin != end);
    auto curr = std::to_integer<std::uint8_t>(*begin);
    bool continuation = curr & 0b10000000;
    std::make_unsigned_t<Integer> material = curr & 0b01111111;
    if (!continuation && !material) {
      if (i) {
        ec = make_error_code(detail::varint_error::overlong);
      } else {
        ++begin;
      }
      return std::pair(Integer(0),
                       begin);
    }
    auto tmp = material;
    std::make_unsigned_t<Integer> shift = 7 * i;
    material <<= shift;
    if ((material >> shift) != tmp) {
      ec = make_error_code(detail::varint_error::overflow);
      return std::pair(Integer(u),
                       begin);
    }
    u |= material;
    ++begin;
    if (!continuation) {
      return std::pair(Integer(u),
                       begin);
    }
    if (begin == end) {
      ec = make_error_code(detail::varint_error::eof);
      return std::pair(Integer(0),
                       begin);
    }
  }
  ec = make_error_code(detail::varint_error::max);
  return std::pair(Integer(0),
                   begin);
}

/**
 *  Attempts to obtain a signed integer from a ZigZag varint
 *  representation stored in a range of `std::byte` objects.
 *
 *  \tparam Integer
 *    The type of signed integer.
 *  \tparam InputIterator
 *    A type which models `InputIterator` and which may be dereferenced
 *    to obtain a `std::byte` value.
 *
 *  \param [in] begin
 *    An iterator to the first `std::byte` to consume.
 *  \param [in] end
 *    An iterator to one past the last `std::byte` to consume.
 *  \param [out] ec
 *    A `std::error_code` object which shall receive the result
 *    of the parse.
 *
 *  \return
 *    A `std::pair` containing the parsed signed integer as the first element,
 *    and an iterator to one past the last `std::byte` consumed as the
 *    second element. If `ec` is truthy after this function returns the
 *    the first element must be disregarded and the second element contains
 *    an iterator to the `std::byte` which provoked the error.
 */
template<typename Integer,
         typename InputIterator>
std::pair<Integer,
          InputIterator> from_zig_zag_varint(InputIterator begin,
                                             InputIterator end,
                                             std::error_code& ec)
{
  static_assert(std::is_signed_v<Integer>);
  auto pair = serialization::from_varint<std::make_unsigned_t<Integer>>(begin,
                                                                        end,
                                                                        ec);
  if (ec) {
    return std::pair(Integer(0),
                     pair.second);
  }
  Integer i(pair.first / 2);
  if (pair.first % 2) {
    i *= -1;
    --i;
  }
  return std::pair(i,
                   pair.second);
}

}
