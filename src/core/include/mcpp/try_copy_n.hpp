/**
 *  \file
 */

#pragma once

#include <cstddef>
#include <iterator>
#include <system_error>
#include <type_traits>
#include <utility>

namespace mcpp {

namespace detail {

enum class try_copy_n_error {
  success = 0,
  eof,
  negative
};

std::error_code make_error_code(try_copy_n_error) noexcept;

template<typename RandomAccessIterator,
         typename Size,
         typename OutputIterator>
std::pair<RandomAccessIterator,
          OutputIterator> try_copy_n(RandomAccessIterator begin,
                                     RandomAccessIterator end,
                                     Size count,
                                     OutputIterator out,
                                     std::error_code& ec,
                                     const std::random_access_iterator_tag&)
{
  ec.clear();
  std::size_t rem(end - begin);
  if (rem < count) {
    ec = make_error_code(try_copy_n_error::eof);
    return std::pair(end,
                     out);
  }
  auto last = begin + count;
  return std::pair(last,
                   std::copy(begin,
                             last,
                             out));
}

template<typename InputIterator,
         typename Size,
         typename OutputIterator>
std::pair<InputIterator,
          OutputIterator> try_copy_n(InputIterator begin,
                                     InputIterator end,
                                     Size count,
                                     OutputIterator out,
                                     std::error_code& ec,
                                     const std::input_iterator_tag&)
{
  ec.clear();
  std::size_t i = 0;
  for (; (i < count) && (begin != end); ++begin, ++i) {
    *(out++) = *begin;
  }
  if (i != count) {
    ec = make_error_code(try_copy_n_error::eof);
  }
  return std::pair(begin,
                   out);
}

}

/**
 *  Attempts to copy a certain number of elements
 *  from a range.
 *
 *  If the correct number of elements cannot be
 *  copied (because the end of the input range is
 *  encountered) an error is generated.
 *
 *  \tparam InputIterator
 *    A type which models `InputIterator`.
 *  \tparam Size
 *    An integer type.
 *  \tparam OutputIterator
 *    A type which models `OutputIterator`.
 *
 *  \param [in] begin
 *    An iterator to the first element to copy.
 *  \param [in] end
 *    An iterator to one past the last element to
 *    copy.
 *  \param [in] count
 *    The number of elements to attempt to copy. If
 *    negative then `ec` will be truthy after the
 *    return and no elements shall be copied.
 *  \param [in] out
 *    An iterator through which each copied element
 *    shall be written.
 *  \param [out] ec
 *    A `std::error_code` which shall receive the
 *    result of the operation.
 *
 *  \return
 *    A `std::pair` whose first element is an
 *    `InputIterator` to one past the last element
 *    copied and whose second element is `out` after
 *    being incremented for each copied element.
 */
template<typename InputIterator,
         typename Size,
         typename OutputIterator>
std::pair<InputIterator,
          OutputIterator> try_copy_n(InputIterator begin,
                                     InputIterator end,
                                     Size count,
                                     OutputIterator out,
                                     std::error_code& ec)
{
  if constexpr (std::is_signed_v<Size>) {
    if (count < 0) {
      ec = make_error_code(detail::try_copy_n_error::negative);
      return std::pair(begin,
                       out);
    }
  }
  typename std::iterator_traits<InputIterator>::iterator_category tag;
  std::make_unsigned_t<Size> size(count);
  return detail::try_copy_n(begin,
                            end,
                            size,
                            out,
                            ec,
                            tag);
}

}
