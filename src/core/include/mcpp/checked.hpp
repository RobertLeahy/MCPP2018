/**
 *  \file
 */

#pragma once

#include <cassert>
#include <limits>
#include <optional>
#include <type_traits>
#include "remove_optional.hpp"

namespace mcpp {

namespace detail {

template<typename... Args>
using checked_promoted_t = std::common_type_t<remove_optional_t<Args>...>;
template<typename... Args>
using checked_promoted_unsigned_t = std::make_unsigned_t<checked_promoted_t<Args...>>;
template<typename... Args>
using checked_promoted_signed_t = std::make_signed_t<checked_promoted_t<Args...>>;

}

/**
 *  Safely converts one integer type to another.
 *
 *  \tparam To
 *    The type to convert to.
 *  \tparam From
 *    The type to convert from.
 *
 *  \param [in] from
 *    The integer value to convert.
 *
 *  \return
 *    A `std::optional` containing an integer of type `To`
 *    if and only if `from` is losslessly representable as
 *    type `To`, an empty `std::optional` otherwise.
 */
template<typename To,
         typename From>
constexpr std::optional<To> checked_cast(From from) noexcept {
  if constexpr (std::is_signed_v<From>) {
    if (from < 0) {
      if constexpr (std::is_signed_v<To>) {
        using promoted_signed_type = detail::checked_promoted_signed_t<To,
                                                                       From>;
        constexpr const promoted_signed_type from_min(std::numeric_limits<From>::min());
        constexpr const promoted_signed_type to_min(std::numeric_limits<To>::min());
        if constexpr (to_min > from_min) {
          const promoted_signed_type promoted_from(from);
          if (promoted_from < to_min) {
            return std::nullopt;
          }
        }
        return To(from);
      } else {
        return std::nullopt;
      }
    }
  }
  using promoted_unsigned_type = detail::checked_promoted_unsigned_t<To,
                                                                     From>;
  constexpr const promoted_unsigned_type from_max(std::numeric_limits<From>::max());
  constexpr const promoted_unsigned_type to_max(std::numeric_limits<To>::max());
  if constexpr (to_max < from_max) {
    const promoted_unsigned_type promoted_from(from);
    if (promoted_from > to_max) {
      return std::nullopt;
    }
  }
  return To(from);
}
/**
 *  Safely converts one integer type to another.
 *
 *  \tparam To
 *    The type to convert to.
 *  \tparam From
 *    The type to convert from.
 *
 *  \param [in] from
 *    A `std::optional` which may contain a value to
 *    convert.
 *
 *  \return
 *    A `std::optional` containing an integer of type `To`
 *    if and only if `from` contains an integer value and that
 *    integer value is losslessly representable as type `To`,
 *    an empty `std::optional` otherwise.
 */
template<typename To,
         typename From>
constexpr std::optional<To> checked_cast(std::optional<From> from) noexcept {
  if (!from) {
    return std::nullopt;
  }
  return mcpp::checked_cast<To>(*from);
}

namespace detail {

template<typename T>
constexpr bool checked_has_value(const T&) noexcept {
  return true;
}
template<typename T>
constexpr bool checked_has_value(const std::optional<T>& opt) noexcept {
  return !!opt;
}

template<typename... Args>
constexpr bool checked_all_have_value(const Args&... args) noexcept {
  return (detail::checked_has_value(args) && ...);
}

template<typename T>
constexpr T checked_unwrap(T t) noexcept {
  return t;
}
template<typename T>
constexpr T checked_unwrap(std::optional<T> t) noexcept {
  assert(t);
  return *t;
}

template<typename First>
constexpr std::optional<First> checked_add(First first) noexcept {
  return first;
}
template<typename First,
         typename Second>
constexpr std::optional<checked_promoted_t<First,
                                           Second>> checked_add(First first,
                                                                Second second) noexcept
{
  using result_type = checked_promoted_t<First,
                                         Second>;
  auto f = mcpp::checked_cast<result_type>(first);
  if (!f) {
    return std::nullopt;
  }
  auto s = mcpp::checked_cast<result_type>(second);
  if (!s) {
    return std::nullopt;
  }
  if constexpr (std::is_signed_v<result_type>) {
    constexpr const auto min = std::numeric_limits<result_type>::min();
    if (*f < 0) {
      if ((min - *f) > *s) {
        return std::nullopt;
      }
      return *f + *s;
    }
    if (*s < 0) {
      assert((min - *s) <= *f);
      return *f + *s;
    }
  }
  constexpr const auto max = std::numeric_limits<result_type>::max();
  if ((max - *s) < *f) {
    return std::nullopt;
  }
  return *f + *s;
}
template<typename First,
         typename Second,
         typename... Args>
constexpr std::optional<checked_promoted_t<First,
                                           Second,
                                           Args...>> checked_add(First first,
                                                                 Second second,
                                                                 Args... args) noexcept
{
  auto opt = detail::checked_add(first,
                                 second);
  if (!opt) {
    return std::nullopt;
  }
  return detail::checked_add(*opt,
                             args...);
}

}

/**
 *  Adds any number of integers of any type producing a result only
 *  if that result is precisely representable.
 *
 *  Heterogeneous types are supported, and the final result (if one exists)
 *  is of the type yielded by performing the usual arithmetic conversions
 *  considering all such types. The operation proceeds from left to right
 *  (i.e. a left fold) and at each step the result is computed only
 *  after converting each operand to the type obtained by the usual arithmetic
 *  conversions against both such operands as if via \ref checked_cast. So
 *  for example given:
 *  \code
 *  unsigned a = 1;
 *  int b = 2;
 *  unsigned long c = 3;
 *  long d = 4;
 *  checked_add(a, b, c, d);
 *  \endcode
 *  Then the computation will be performed as if by:
 *  \code
 *  //  Since the usual arithmetic conversions against
 *  //  int and unsigned yields unsigned
 *  unsigned a(1);
 *  unsigned b(2);
 *  //  Since the usual arithmetic conversions against
 *  //  unsigned and unsigned long yields unsigned long
 *  unsigned long ab(a + b);
 *  unsigned long c(3);
 *  //  Since the usual arithmetic conversions against
 *  //  long and unsigned long yields unsigned long
 *  unsigned long abc(ab + c);
 *  unsigned long d(4);
 *  //  The final result is of type unsigned long since
 *  //  that's the result of the usual arithmetic conversions
 *  //  against int, unsigned, unsigned long, and long
 *  unsigned long abcd(abc + 4);
 *  \endcode
 *
 *  Integers wrapped in `std::optional` are handled transparently: Any
 *  parameter which is a `std::optional` which is empty will result in
 *  the overall operation ending immediately with an empty `std::optional`
 *  as a result, otherwise the operation proceeds as if the result of
 *  `std::optional::operator*` had been provided instead.
 *
 *  \tparam Args
 *    The types of the arguments. `sizeof...(Args)` must be at least
 *    `1` or the program is ill-formed.
 *
 *  \param [in] args
 *    The arguments to add.
 *
 *  \return
 *    The result of the addition if that value is representable in
 *    `std::common_type_t<Args...>` (after removing `std::optional` wrappers
 *    from each type in `Args`), an empty `std::optional` otherwise.
 */
template<typename... Args>
constexpr std::optional<std::common_type_t<remove_optional_t<Args>...>> checked_add(Args... args) noexcept {
  if (!detail::checked_all_have_value(args...)) {
    return std::nullopt;
  }
  return detail::checked_add(detail::checked_unwrap(args)...);
}

namespace detail {

template<typename First>
constexpr std::optional<First> checked_multiply(First first) noexcept {
  return first;
}
template<typename First,
         typename Second>
constexpr std::optional<checked_promoted_t<First,
                                           Second>> checked_multiply(First first,
                                                                     Second second) noexcept
{
  using result_type = checked_promoted_t<First,
                                         Second>;
  auto f = mcpp::checked_cast<result_type>(first);
  if (!f) {
    return std::nullopt;
  }
  auto s = mcpp::checked_cast<result_type>(second);
  if (!s) {
    return std::nullopt;
  }
  auto tmp_f = *f;
  auto tmp_s = *s;
  if constexpr (std::is_signed_v<result_type>) {
    constexpr const auto min = std::numeric_limits<result_type>::min();
    //  This is a hacky way to work around the asymmetric
    //  of 2's complement numbers (maybe in C++20 when
    //  signed numbers are 2's complement this won't be
    //  a hack anymore)
    if (*f == min) {
      switch (*s) {
      case 0:
        return 0;
      case 1:
        return *f;
      }
      return std::nullopt;
    }
    if (*s == min) {
      switch (*f) {
      case 0:
        return 0;
      case 1:
        return *s;
      }
      return std::nullopt;
    }
    if (tmp_f < 0) {
      tmp_f *= -1;
    }
    if (tmp_s < 0) {
      tmp_s *= -1;
    }
  }
  constexpr const auto max = std::numeric_limits<result_type>::max();
  if ((max / tmp_s) < tmp_f) {
    return std::nullopt;
  }
  return *f * *s;
}
template<typename First,
         typename Second,
         typename... Args>
constexpr std::optional<checked_promoted_t<First,
                                           Second,
                                           Args...>> checked_multiply(First first,
                                                                      Second second,
                                                                      Args... args) noexcept
{
  auto opt = detail::checked_multiply(first,
                                      second);
  if (!opt) {
    return std::nullopt;
  }
  return detail::checked_multiply(*opt,
                                  args...);
}

}

/**
 *  Multiplies any number of integers of any type producing a result
 *  if that result is precisely representable.
 *
 *  The operation proceeds in the same manner as \ref checked_add.
 *
 *  \tparam Args
 *    The types of arguments. `sizeof...(Args)` must be at least
 *    `1` or the program is ill-formed.
 *
 *  \param [in] args
 *    The arguments to add.
 *
 *  \return
 *    The result of the multiplication if that value is representable in
 *    `std::common_type_t<Args...>` (after removing `std::optional` wrappers
 *    from each type in `Args`), an empty `std::optional` otherwise.
 */
template<typename... Args>
constexpr std::optional<std::common_type_t<remove_optional_t<Args>...>> checked_multiply(Args... args) noexcept {
  if (!detail::checked_all_have_value(args...)) {
    return std::nullopt;
  }
  return detail::checked_multiply(detail::checked_unwrap(args)...);
}

}
