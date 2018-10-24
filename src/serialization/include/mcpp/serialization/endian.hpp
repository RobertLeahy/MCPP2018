/**
 *  \file
 */

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <system_error>
#include <utility>
#include <type_traits>
#include <boost/endian/conversion.hpp>
#include <mcpp/try_copy_n.hpp>

namespace mcpp::serialization {

/**
 *  Converts an object (which must be a `TrivialType`) to
 *  bytes by adjusting its endianness to network byte order
 *  (i.e. big endian) and then simply bitwise copying it to
 *  the output.
 *
 *  \tparam Trivial
 *    A type which is a `TrivialType`.
 *  \tparam OutputIterator
 *    A type which models `OutputIterator`.
 *
 *  \param [in] val
 *    The value to serialize.
 *  \param [in] out
 *    The `OutputIterator` through which the bytes of the
 *    representation of `val` shall be written.
 *
 *  \return
 *    A copy of `out` which has been incremented an appropriate
 *    number of times.
 */
template<typename Trivial,
         typename OutputIterator>
OutputIterator to_endian(Trivial val,
                         OutputIterator out)
{
  static_assert(std::is_trivial<Trivial>::value);
  std::array<std::byte,
             sizeof(Trivial)> arr;
  std::memcpy(arr.data(),
              &val,
              sizeof(val));
  if constexpr (boost::endian::order::big != boost::endian::order::native) {
    std::reverse(arr.begin(),
                 arr.end());
  }
  return std::copy(arr.begin(),
                   arr.end(),
                   out);
}

/**
 *  Retrieves an object (which must be a `TrivialType`) from a
 *  sequence of bytes by copying an appropriate number of bytes,
 *  adjusting their endianness (from network byte order) and then
 *  beginnning the lifetime of the deserialized object through
 *  `std::memcpy` (as is permitted for objects of types which are
 *  a `TrivialType`).
 *
 *  \tparam Trivial
 *    A type which is a `TrivialType`.
 *  \tparam InputIterator
 *    A type which models `InputIterator`.
 *
 *  \param [in] begin
 *    An iterator to the first `std::byte` to consume.
 *  \param [in] end
 *    An iterator to one past the last `std::byte` to consume.
 *  \param [out] ec
 *    A `std::error_code` which shall receive the result of the
 *    operation.
 *
 *  \return
 *    A `std::pair` whose first element is the deserialized object
 *    and whose second element is a copy of `begin` incremented past
 *    the representation of that object. If `ec` is truthy after the
 *    return the first element must be disregarded and the second
 *    element points to the `std::byte` which provoked the error.
 */
template<typename Trivial,
         typename InputIterator>
std::pair<Trivial,
          InputIterator> from_endian(InputIterator begin,
                                     InputIterator end,
                                     std::error_code& ec)
{
  ec.clear();
  std::array<std::byte,
             sizeof(Trivial)> arr;
  begin = mcpp::try_copy_n(begin,
                           end,
                           arr.size(),
                           arr.begin(),
                           ec).first;
  std::pair retr(Trivial(),
                 begin);
  if (ec) {
    return retr;
  }
  std::reverse(arr.begin(),
               arr.end());
  std::memcpy(&retr.first,
              arr.data(),
              arr.size());
  return retr;
}

}
