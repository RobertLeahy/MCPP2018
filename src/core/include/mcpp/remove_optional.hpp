/**
 *  \file
 */

#pragma once

#include <optional>

namespace mcpp {

/**
 *  Extracts the type wrapped by `std::optional` ignoring
 *  top-level cv-qualification. If the type is not wrapped by
 *  `std::optional` then this is the identity metafunction.
 *  If the type is wrapped in multiple levels of `std::optional`
 *  only the topmost is removed.
 *
 *  \tparam T
 *    The type.
 */
template<typename T>
class remove_optional {
public:
  /**
   *  The resulting type. If `T` is `std::optional<U>`,
   *  `const std::optional<U>`, `volatile std::optional<U>`,
   *  or `const volatile std::optional<U>` then this is `U`,
   *  otherwise `T`.
   */
  using type = T;
};
#ifndef MCPP_DOXYGEN_RUNNING
template<typename T>
class remove_optional<std::optional<T>> {
public:
  using type = T;
};
template<typename T>
class remove_optional<const std::optional<T>> {
public:
  using type = T;
};
template<typename T>
class remove_optional<volatile std::optional<T>> {
public:
  using type = T;
};
template<typename T>
class remove_optional<const volatile std::optional<T>> {
public:
  using type = T;
};
#endif

/**
 *  A convenience type alias for the result of the
 *  \ref remove_optional metafunction.
 *
 *  \tparam T
 *    The type to pass through to \ref remove_optional.
 */
template<typename T>
using remove_optional_t = typename remove_optional<T>::type;

}
