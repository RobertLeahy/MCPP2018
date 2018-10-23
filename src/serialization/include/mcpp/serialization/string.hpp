/**
 *  \file
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include "varint.hpp"

namespace mcpp::serialization {

/**
 *  The maximum length of the Minecraft protocol
 *  representation of a string.
 */
constexpr std::size_t string_max_size = varint_size(std::numeric_limits<std::int16_t>::max()) +
                                        std::size_t(std::numeric_limits<std::int16_t>::max());

/**
 *  Checks to see if a string of a certain length
 *  is representable in the Minecraft protocol.
 *
 *  \param [in] size
 *    The size of the string.
 *
 *  \return
 *    `true` if a string of the size-in-question can
 *    be represented, `false` otherwise.
 */
inline bool string_check(std::size_t size) noexcept {
  constexpr std::size_t max(std::numeric_limits<std::int16_t>::max());
  return size <= max;
}
/**
 *  Checks to see if a string is representable in
 *  the Minecraft protocol.
 *
 *  \tparam InputIterator
 *    A type which models `InputIterator`.
 *
 *  \param [in] begin
 *    An iterator to the first character in the
 *    string.
 *  \param [in] end
 *    An iterator to one past the last character in
 *    the string.
 *
 *  \return
 *    `true` if the string can be represented, `false`
 *    otherwise.
 */
template<typename InputIterator>
bool string_check(InputIterator begin,
                  InputIterator end) noexcept(noexcept(std::distance(begin,
                                                                     end)))
{
  std::size_t size(std::distance(begin,
                                 end));
  return string_check(size);
}
/**
 *  Checks to see if a string is representable in
 *  the Minecraft protocol.
 *
 *  \tparam CharT
 *    The character type.
 *  \tparam Traits
 *    The character traits type.
 *
 *  \param [in] sv
 *    A `std::basic_string_view` to the string.
 *
 *  \return
 *    `true` if the string can be represented, `false`
 *    otherwise.
 */
template<typename CharT,
         typename Traits>
bool string_check(std::basic_string_view<CharT,
                                         Traits> sv) noexcept
{
  return serialization::string_check(sv.begin(),
                                     sv.end());
}
/**
 *  Checks to see if a string is representable in
 *  the Minecraft protocol.
 *
 *  \param [in] str
 *    A NTBS.
 *
 *  \return
 *    `true` if the string can be represented, `false`
 *    otherwise.
 */
inline bool string_check(const char* str) noexcept {
  std::string_view sv(str);
  return serialization::string_check(sv);
}
/**
 *  Checks to see if a string is representable in
 *  the Minecraft protocol.
 *
 *  \tparam CharT
 *    The character type.
 *  \tparam Traits
 *    The character traits type.
 *  \tparam Allocator
 *    A type which models `Allocator`.
 *
 *  \param [in] str
 *    The string.
 *
 *  \return
 *    `true` if the string can be represented, `false`
 *    otherwise.
 */
template<typename CharT,
         typename Traits,
         typename Allocator>
bool string_check(const std::basic_string<CharT,
                                          Traits,
                                          Allocator>& str) noexcept
{
  std::basic_string_view<CharT,
                         Traits> sv(str);
  return serialization::string_check(sv);
}

/**
 *  Determines the number of bytes required to represent
 *  a certain string.
 *
 *  If the string cannot be represented in the Minecraft
 *  protocol an exception is thrown.
 *
 *  \tparam InputIterator
 *    A type which models `InputIterator`.
 *
 *  \param [in] begin
 *    An iterator to the first character in the
 *    string.
 *  \param [in] end
 *    An iterator to one past the last character in
 *    the string.
 *
 *  \return
 *    The number of bytes required to represent the
 *    string.
 */
template<typename InputIterator>
std::size_t string_size(InputIterator begin,
                        InputIterator end)
{
  static_assert(sizeof(*begin) == 1);
  std::size_t size(std::distance(begin,
                                 end));
  if (!string_check(size)) {
    throw std::system_error(make_error_code(std::errc::value_too_large));
  }
  return size + varint_size(size);
}
/**
 *  Determines the number of bytes required to represent
 *  a certain string.
 *
 *  If the string cannot be represented in the Minecraft
 *  protocol an exception is thrown.
 *
 *  \tparam CharT
 *    The character type.
 *  \tparam Traits
 *    The character traits type.
 *
 *  \param [in] sv
 *    A `std::basic_string_view` encapsulating the string.
 *
 *  \return
 *    The number of bytes required to represent the
 *    string.
 */
template<typename CharT,
         typename Traits>
std::size_t string_size(std::basic_string_view<CharT,
                                               Traits> sv)
{
  return serialization::string_size(sv.begin(),
                                    sv.end());
}
/**
 *  Determines the number of bytes required to represent
 *  a certain string.
 *
 *  If the string cannot be represented in the Minecraft
 *  protocol an exception is thrown.
 *
 *  \param [in] str
 *    A NTBS.
 *
 *  \return
 *    The number of bytes required to represent the
 *    string.
 */
inline std::size_t string_size(const char* str) {
  std::string_view sv(str);
  return serialization::string_size(sv);
}
/**
 *  Determines the number of bytes required to represent
 *  a certain string.
 *
 *  If the string cannot be represented in the Minecraft
 *  protocol an exception is thrown.
 *
 *  \tparam CharT
 *    The character type.
 *  \tparam Traits
 *    The character traits type.
 *  \tparam Allocator
 *    A type which models `Allocator`.
 *
 *  \param [in] str
 *    The string.
 *
 *  \return
 *    The number of bytes required to represent the
 *    string.
 */
template<typename CharT,
         typename Traits,
         typename Allocator>
std::size_t string_size(const std::basic_string<CharT,
                                                Traits,
                                                Allocator>& str)
{
  std::basic_string_view<CharT,
                         Traits> sv(str);
  return serialization::string_size(sv);
}

/**
 *  Serializes a string to the representation required
 *  by the Minecraft wire protocol.
 *
 *  \tparam ForwardIterator
 *    A type which models `ForwardIterator`.
 *  \tparam OutputIterator
 *    A type which models `OutputIterator`.
 *
 *  \param [in] begin
 *    An iterator to the first character to serialize.
 *  \param [in] end
 *    An iterator to one past the last character to
 *    serialize.
 *  \param [in] out
 *    The `OutputIterator` through which `std::byte`
 *    objects representing `sv` shall be written.
 *
 *  \return
 *    `out` after being written to and incremented an
 *    appropriate number of times.
 */
template<typename ForwardIterator,
         typename OutputIterator>
OutputIterator to_string(ForwardIterator begin,
                         ForwardIterator end,
                         OutputIterator out)
{
  static_assert(sizeof(*begin) == 1);
  std::size_t size(std::distance(begin,
                                 end));
  if (!string_check(size)) {
    throw std::system_error(make_error_code(std::errc::value_too_large));
  }
  out = serialization::to_varint(size,
                                 out);
  return std::transform(begin,
                        end,
                        out,
                        [](auto c) noexcept { unsigned char u(c);
                                              return std::byte{u}; });
}
/**
 *  Serializes a string to the representation required
 *  by the Minecraft wire protocol.
 *
 *  \tparam CharT
 *    The character type of the string to serialize.
 *  \tparam Traits
 *    The character traits of the string to serialize.
 *  \tparam OutputIterator
 *    A type which models `OutputIterator`.
 *
 *  \param [in] sv
 *    A `std::basic_string_view` through which the string
 *    to serialize may be accessed.
 *  \param [in] begin
 *    The `OutputIterator` through which `std::byte`
 *    objects representing `sv` shall be written.
 *
 *  \return
 *    `begin` after being written to and incremented an
 *    appropriate number of times.
 */
template<typename CharT,
         typename Traits,
         typename OutputIterator>
OutputIterator to_string(std::basic_string_view<CharT,
                                                Traits> sv,
                         OutputIterator begin)
{
  return serialization::to_string(sv.begin(),
                                  sv.end(),
                                  begin);
}
/**
 *  Serializes a string to the representation required
 *  by the Minecraft wire protocol.
 *
 *  \tparam OutputIterator
 *    A type which models `OutputIterator`.
 *
 *  \param [in] str
 *    A NTBS which shall be serialized.
 *  \param [in] begin
 *    The `OutputIterator` through which `std::byte`
 *    objects representing `sv` shall be written.
 *
 *  \return
 *    `begin` after being written to and incremented an
 *    appropriate number of times.
 */
template<typename OutputIterator>
OutputIterator to_string(const char* str,
                         OutputIterator begin)
{
  return serialization::to_string(std::string_view(str),
                                  begin);
}
/**
 *  Serializes a string to the representation required
 *  by the Minecraft wire protocol.
 *
 *  \tparam CharT
 *    The character type of the string to serialize.
 *  \tparam Traits
 *    The character traits of the string to serialize.
 *  \tparam Allocator
 *    The `Allocator` associated with the string to
 *    serialize.
 *  \tparam OutputIterator
 *    A type which models `OutputIterator`.
 *
 *  \param [in] str
 *    The `std::basic_string` which shall be serialized.
 *  \param [in] begin
 *    The `OutputIterator` through which `std::byte`
 *    objects representing `sv` shall be written.
 *
 *  \return
 *    `begin` after being written to and incremented an
 *    appropriate number of times.
 */
template<typename CharT,
         typename Traits,
         typename Allocator,
         typename OutputIterator>
OutputIterator to_string(const std::basic_string<CharT,
                                                 Traits,
                                                 Allocator>& str,
                         OutputIterator begin)
{
  std::basic_string_view<CharT,
                         Traits> sv(str);
  return serialization::to_string(sv,
                                  begin);
}

namespace detail {

enum class string_error {
  success = 0,
  eof,
  negative_size
};

std::error_code make_error_code(string_error) noexcept;

template<typename CharT,
         typename RandomAccessIterator,
         typename OutputIterator>
std::pair<RandomAccessIterator,
          OutputIterator> from_string_copy(RandomAccessIterator begin,
                                           RandomAccessIterator end,
                                           std::size_t n,
                                           OutputIterator out,
                                           std::error_code& ec,
                                           const std::random_access_iterator_tag&)
{
  ec.clear();
  std::size_t rem(end - begin);
  if (rem < n) {
    ec = make_error_code(string_error::eof);
    return std::pair(end,
                     out);
  }
  auto last = begin + n;
  return std::pair(last,
                   std::transform(begin,
                                  last,
                                  out,
                                  [](auto b) noexcept { return std::to_integer<CharT>(b); }));
}
template<typename CharT,
         typename InputIterator,
         typename OutputIterator>
std::pair<InputIterator,
          OutputIterator> from_string_copy(InputIterator begin,
                                           InputIterator end,
                                           std::size_t n,
                                           OutputIterator out,
                                           std::error_code& ec,
                                           const std::input_iterator_tag&)
{
  ec.clear();
  std::size_t i = 0;
  for (; (i < n) && (begin != end); ++begin, ++i) {
    *(out++) = std::to_integer<CharT>(*begin);
  }
  if (i != n) {
    ec = make_error_code(string_error::eof);
  }
  return std::pair(begin,
                   out);
}
template<typename CharT,
         typename InputIterator,
         typename OutputIterator>
std::pair<InputIterator,
          OutputIterator> from_string_copy(InputIterator begin,
                                           InputIterator end,
                                           std::size_t n,
                                           OutputIterator out,
                                           std::error_code& ec)
{
  ec.clear();
  typename std::iterator_traits<InputIterator>::iterator_category tag;
  return detail::from_string_copy<CharT>(begin,
                                         end,
                                         n,
                                         out,
                                         ec,
                                         tag);
}


}

/**
 *  Parses the Minecroft protocol representation of a string
 *  from a buffer of bytes.
 *
 *  \tparam CharT
 *    The character type of the string to extract.
 *  \tparam InputIterator
 *    A type which models `InputIterator`.
 *  \tparam OutputIterator
 *    A type which models `OutputIterator`.
 *
 *  \param [in] begin
 *    An iterator to the first `std::byte` to parse.
 *  \param [in] end
 *    An iterator to one past the last `std::byte` to parse.
 *  \param [in] out
 *    The iterator through which each parsed character of
 *    type `CharT` shall be written.
 *  \param [out] ec
 *    A `std::error_code` which shall receive the result of
 *    the operation.
 *
 *  \return
 *    A `std::pair` whose first element is `begin` incremented
 *    past the parsed string and whose second element is `out`
 *    incremented for each parsed object of type `CharT`. If
 *    `ec` is truthy after the call succeeds the first value
 *    shall point to the `std::byte` at which the problem was
 *    encountered.
 */
template<typename CharT,
         typename InputIterator,
         typename OutputIterator>
std::pair<InputIterator,
          OutputIterator> from_string(InputIterator begin,
                                      InputIterator end,
                                      OutputIterator out,
                                      std::error_code& ec)
{
  ec.clear();
  auto pair = serialization::from_varint<std::int16_t>(begin,
                                                       end,
                                                       ec);
  begin = pair.second;
  if (ec) {
    return std::pair(begin,
                     out);
  }
  if (pair.first < 0) {
    ec = make_error_code(detail::string_error::negative_size);
    return std::pair(begin,
                     out);
  }
  return detail::from_string_copy<CharT>(begin,
                                         end,
                                         pair.first,
                                         out,
                                         ec);
}
/**
 *  Parses the Minecroft protocol representation of a string
 *  from a buffer of bytes.
 *
 *  \tparam InputIterator
 *    A type which models `InputIterator`.
 *  \tparam CharT
 *    The character type.
 *  \tparam Traits
 *    The character traits type.
 *  \tparam Allocator
 *    A type which models `Allocator`.
 *
 *  \param [in] begin
 *    An iterator to the first `std::byte` to parse.
 *  \param [in] end
 *    An iterator to one past the last `std::byte` to parse.
 *  \param [in] str
 *    A `std::basic_string` whose managed memory and allocator
 *    shall be used to store the returned value.
 *  \param [out] ec
 *    A `std::error_code` which shall receive the result of
 *    the operation.
 *
 *  \return
 *    A `std::pair` whose first element is a `std::basic_string`
 *    containing the parsed string and whose second element is
 *    `begin` incremented for each parsed `std::byte`. If
 *    `ec` is truthy after the call succeeds the second value
 *    shall point to the `std::byte` at which the problem was
 *    encountered.
 */
template<typename InputIterator,
         typename CharT,
         typename Traits,
         typename Allocator>
std::pair<std::basic_string<CharT,
                            Traits,
                            Allocator>,
          InputIterator> from_string(InputIterator begin,
                                     InputIterator end,
                                     std::basic_string<CharT,
                                                       Traits,
                                                       Allocator> str,
                                     std::error_code& ec)
{
  str.clear();
  begin = serialization::from_string<CharT>(begin,
                                            end,
                                            std::back_inserter(str),
                                            ec).first;
  std::pair retr(std::move(str),
                 begin);
  if (ec) {
    retr.first.clear();
  }
  return retr;
}
/**
 *  Parses the Minecroft protocol representation of a string
 *  from a buffer of bytes.
 *
 *  \tparam String
 *    An instantiation of `std::basic_string`.
 *  \tparam InputIterator
 *    A type which models `InputIterator`.
 *
 *  \param [in] begin
 *    An iterator to the first `std::byte` to parse.
 *  \param [in] end
 *    An iterator to one past the last `std::byte` to parse.
 *  \param [out] ec
 *    A `std::error_code` which shall receive the result of
 *    the operation.
 *
 *  \return
 *    A `std::pair` whose first element is a `String`
 *    containing the parsed string and whose second element is
 *    `begin` incremented for each parsed `std::byte`. If
 *    `ec` is truthy after the call succeeds the second value
 *    shall point to the `std::byte` at which the problem was
 *    encountered.
 */
template<typename String,
         typename InputIterator>
std::pair<String,
          InputIterator> from_string(InputIterator begin,
                                     InputIterator end,
                                     std::error_code& ec)
{
  return serialization::from_string(begin,
                                    end,
                                    String(),
                                    ec);
}

}
