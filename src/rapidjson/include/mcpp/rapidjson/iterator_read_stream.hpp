/**
 *  \file
 */

#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace mcpp::rapidjson {

/**
 *  A model of the `Stream` concept from RapidJSON
 *  which reads `char` objects from a model of
 *  `ForwardIterator`.
 *
 *  \tparam ForwardIterator
 *    A type which models `ForwardIterator` instances of
 *    which may be deferenced to obtain an object of
 *    type `char`.
 */
template<typename ForwardIterator>
class iterator_read_stream {
public:
  /**
   *  Creates a new iterator_read_stream from a pair
   *  of iterators.
   *
   *  \param [in] begin
   *    An iterator to the first `char` to read.
   *  \param [in] end
   *    An iterator to one past the last `char` to
   *    read.
   */
  iterator_read_stream(ForwardIterator begin,
                       ForwardIterator end) noexcept(std::is_nothrow_move_constructible_v<ForwardIterator> &&
                                                     std::is_nothrow_copy_constructible_v<ForwardIterator>)
    : begin_(begin),
      curr_ (std::move(begin)),
      end_  (std::move(end))
  {}
#ifndef MCPP_DOXYGEN_RUNNING
  using Ch = char;
private:
  static constexpr bool peek_noexcept = noexcept(*std::declval<const ForwardIterator&>()) &&
                                        noexcept(std::declval<const ForwardIterator&>() == std::declval<const ForwardIterator&>());
public:
  Ch Peek() const noexcept(peek_noexcept) {
    if (curr_ == end_) {
      return '\0';
    }
    return *curr_;
  }
  Ch Take() noexcept(peek_noexcept &&
                     noexcept(++std::declval<ForwardIterator&>()))
  {
    auto retr = Peek();
    if (!retr) {
      return retr;
    }
    ++curr_;
    return retr;
  }
  std::size_t Tell() const noexcept(noexcept(std::distance(std::declval<const ForwardIterator&>(),
                                                           std::declval<const ForwardIterator&>())))
  {
    return std::size_t(std::distance(curr_,
                                     end_));
  }
  Ch* PutBegin() const noexcept {
    return nullptr;
  }
  void Put(Ch) const noexcept {}
  void Flush() const noexcept {}
  std::size_t PutEnd(Ch*) const noexcept {
    return 0;
  }
#endif
private:
  ForwardIterator begin_;
  ForwardIterator curr_;
  ForwardIterator end_;
};

}
