/**
 *  \file
 */

#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

namespace mcpp {

/**
 *  An `OutputIterator` which performs a cast to
 *  type `T` before writing through to an underlying
 *  `OutputIterator`.
 *
 *  \tparam T
 *    The type to cast written values to.
 *  \tparam OutputIterator
 *    The type of the underlying `OutputIterator`.
 */
template<typename T,
         typename OutputIterator>
class cast_output_iterator {
public:
  /**
   *  @{
   *
   *  As required for `std::iterator_traits`.
   */
  using iterator_category = std::output_iterator_tag;
  using value_type = void;
  using difference_type = void;
  using pointer = void;
  using reference = void;
  /**
   *  @}
   *
   *  Creates a cast_output_iterator which wraps an
   *  `OutputIterator`.
   *
   *  \param [in] iter
   *    The `OutputIterator` to wrap.
   */
  explicit cast_output_iterator(OutputIterator iter) noexcept(std::is_nothrow_move_constructible_v<OutputIterator>)
    : iter_(std::move(iter))
  {}
#ifndef MCPP_DOXYGEN_RUNNING
  cast_output_iterator(const cast_output_iterator&) = default;
  cast_output_iterator(cast_output_iterator&&) = default;
  cast_output_iterator& operator=(const cast_output_iterator&) = default;
  cast_output_iterator& operator=(cast_output_iterator&&) = default;
#endif
  /**
   *  Writes to the underlying iterator.
   *
   *  \tparam U
   *    The type to write.
   *
   *  \param [in] u
   *    The value to write.
   *
   *  \return
   *    A reference to this object.
   */
  template<typename U>
#ifdef MCPP_DOXYGEN_RUNNING
  cast_output_iterator&
#else
  std::enable_if_t<!std::is_same_v<std::decay_t<U>,
                                   cast_output_iterator>,
                   cast_output_iterator&> 
#endif
  operator=(U&& u) noexcept(std::is_nothrow_constructible_v<T,
                                                            U&&> &&
                            noexcept(*std::declval<OutputIterator>() = std::declval<T>()))
  {
    *iter_ = T(std::forward<U>(u));
    return *this;
  }
  /**
   *  Does nothing.
   *
   *  \return
   *    A reference to this object.
   */
  cast_output_iterator& operator*() noexcept {
    return *this;
  }
  /**
   *  Increments the underlying iterator.
   *
   *  \param [in] ignored
   *    Ignored.
   *
   *  \return
   *    A copy of this object before the underlying
   *    iterator was incremented.
   */
  cast_output_iterator operator++(int ignored) noexcept(std::is_nothrow_copy_constructible_v<OutputIterator> &&
                                                        noexcept(++std::declval<OutputIterator&>()))
  {
    (void)ignored;
    cast_output_iterator retr(*this);
    ++*this;
    return retr;
  }
  /**
   *  Increments the underlying iterator.
   *
   *  \return
   *    A reference to this object.
   */
  cast_output_iterator& operator++() noexcept(noexcept(++std::declval<OutputIterator&>())) {
    ++iter_;
    return *this;
  }
  /**
   *  Retrieves a copy of the underlying iterator.
   *
   *  \return
   *    A copy of the underlying iterator.
   */
  OutputIterator base() const noexcept(std::is_nothrow_copy_constructible_v<OutputIterator>) {
    return iter_;
  }
private:
  OutputIterator iter_;
};

/**
 *  Creates a \ref cast_output_iterator allowing for
 *  the second template parameter to be deduced.
 *
 *  \tparam T
 *    The type which each output value shall be cast
 *    to.
 *  \tparam OutputIterator
 *    A type which models `OutputIterator`.
 *
 *  \param [in] iter
 *    The `OutputIterator` to wrap.
 *
 *  \return
 *    A \ref cast_output_iterator which wraps `iter`.
 */
template<typename T,
         typename OutputIterator>
auto make_cast_output_iterator(OutputIterator iter) noexcept(std::is_nothrow_constructible_v<cast_output_iterator<T,
                                                                                                                  OutputIterator>,
                                                                                             OutputIterator>)
{
  return cast_output_iterator<T,
                              OutputIterator>(std::move(iter));
}

}
