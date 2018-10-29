/**
 *  \file
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <mcpp/cast_output_iterator.hpp>
#include <mcpp/serialization/endian.hpp>

namespace mcpp::nbt {

namespace detail {

[[noreturn]]
void iterator_sax_writer_string_length_error(std::size_t);

}

/**
 *  Writes SAX-like events to NBT via an
 *  `OutputIterator`.
 *
 *  Models the `SaxWriter` concept which is identical
 *  to the `SaxObserver` concept required by the
 *  \ref sax_parse function except:
 *  - There are no type aliases `allocator_type` or
 *    `string_type`
 *  - There is no member template type alias `vector_type`
 *  - There is no member `get_allocator`
 *  - There is a member type alias `string_view_type`
 *    which is an alias for an instantiation of
 *    `std::basic_string_view`
 *  - All member functions which accept arguments of type
 *    `iter` are present except they do not accept those
 *    arguments
 *  - Member functions do not return `std::error_code`
 *  - There is no `error` member function
 *
 *  \tparam OutputIterator
 *    A model of `OutputIterator` to which instances
 *    of `std::byte` may be written.
 */
template<typename OutputIterator>
class iterator_sax_writer {
public:
  /**
   *  Type alias for the `OutputIterator` template
   *  parameter.
   */
  using iterator_type = OutputIterator;
  /**
   *  Creates a new iterator_sax_writer which writes
   *  NBT through a certain `OutputIterator`.
   *
   *  \param [in] iter
   *    The iterator.
   */
  explicit iterator_sax_writer(OutputIterator iter) noexcept(std::is_nothrow_move_constructible_v<iterator_type>)
    : iter_(std::move(iter))
  {}
  /**
   *  Retrives the wrapped `OutputIterator`.
   *
   *  \return
   *    The iterator.
   */
  iterator_type iterator() const noexcept(std::is_nothrow_copy_constructible_v<iterator_type>) {
    return iter_;
  }
#ifndef MCPP_DOXYGEN_RUNNING
  using allocator_type = std::allocator<void>;
  allocator_type get_allocator() const noexcept {
    return allocator_type();
  }
  using string_type = std::string;
  template<typename T>
  using vector_type = std::vector<T>;
  using string_view_type = std::string_view;
  template<typename... Args>
  std::error_code begin(Args&&...) const noexcept {
    return std::error_code();
  }
  template<typename... Args>
  std::error_code end(Args&&...) const noexcept {
    return std::error_code();
  }
  template<typename... Args>
  std::error_code begin_compound(Args&&...) const noexcept {
    return std::error_code();
  }
  template<typename... Args>
  std::error_code end_compound(Args&&...) const noexcept {
    return std::error_code();
  }
  template<typename... Args>
  std::error_code begin_list(Args&&...) const noexcept {
    return std::error_code();
  }
  template<typename... Args>
  std::error_code end_list(Args&&...) const noexcept {
    return std::error_code();
  }
  template<typename... Args>
  std::error_code begin_byte_array(Args&&...) const noexcept {
    return std::error_code();
  }
  template<typename... Args>
  std::error_code end_byte_array(Args&&...) const noexcept {
    return std::error_code();
  }
  template<typename... Args>
  std::error_code begin_int_array(Args&&...) const noexcept {
    return std::error_code();
  }
  template<typename... Args>
  std::error_code end_int_array(Args&&...) const noexcept {
    return std::error_code();
  }
  template<typename... Args>
  std::error_code begin_long_array(Args&&...) const noexcept {
    return std::error_code();
  }
  template<typename... Args>
  std::error_code end_long_array(Args&&...) const noexcept {
    return std::error_code();
  }
  template<typename... Args>
  std::error_code value(std::int8_t value,
                        Args&&...)
  {
    *iter_ = std::byte(value);
    ++iter_;
    return std::error_code();
  }
  template<typename... Args>
  std::error_code value(std::int16_t value,
                        Args&&...)
  {
    endian(value);
    return std::error_code();
  }
  template<typename... Args>
  std::error_code value(std::int32_t value,
                        Args&&...)
  {
    endian(value);
    return std::error_code();
  }
  template<typename... Args>
  std::error_code value(std::int64_t value,
                        Args&&...)
  {
    endian(value);
    return std::error_code();
  }
  template<typename... Args>
  std::error_code value(float value,
                        Args&&...)
  {
    endian(value);
    return std::error_code();
  }
  template<typename... Args>
  std::error_code value(double value,
                        Args&&...)
  {
    endian(value);
    return std::error_code();
  }
  template<typename... Args>
  std::error_code value(string_view_type sv,
                        Args&&...)
  {
    constexpr string_view_type::size_type max(std::numeric_limits<std::uint16_t>::max());
    if (sv.size() > max) {
      detail::iterator_sax_writer_string_length_error(sv.size());
    }
    endian(std::uint16_t(sv.size()));
    iter_ = std::copy(sv.begin(),
                      sv.end(),
                      mcpp::make_cast_output_iterator<std::byte>(iter_)).base();
    return std::error_code();
  }
  template<typename... Args>
  std::error_code name(string_view_type sv,
                       Args&&...)
  {
    value(sv);
    return std::error_code();
  }
  template<typename... Args>
  std::error_code length(std::int32_t length,
                         Args&&...)
  {
    value(length);
    return std::error_code();
  }
  template<typename... Args>
  std::error_code tag(std::byte tag,
                      Args&&...)
  {
    assert(std::to_integer<std::uint8_t>(tag) <= 12);
    *iter_ = tag;
    ++iter_;
    return std::error_code();
  }
  template<typename... Args>
  void error(Args&&...) const noexcept {}
#endif
private:
  template<typename Value>
  void endian(Value v) {
    iter_ = serialization::to_endian(v,
                                     iter_);
  }
  iterator_type iter_;
};

}
