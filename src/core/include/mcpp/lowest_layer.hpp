/**
 *  \file
 */

#pragma once

#include <type_traits>

namespace mcpp {

namespace detail {

template<typename T,
         typename = void>
class has_lowest_layer : public std::bool_constant<false> {};
template<typename T>
class has_lowest_layer<T,
                       std::void_t<typename T::lowest_layer_type>> : public std::bool_constant<true> {};

template<typename T,
         typename = void>
class lowest_layer {
public:
  using type = T;
};
template<typename T>
class lowest_layer<T,
                   std::void_t<typename T::lowest_layer_type>>
{
public:
  using type = typename T::lowest_layer_type;
};

template<typename T>
decltype(auto) get_lowest_layer(T&& t,
                                std::true_type) noexcept
{
  return t.lowest_layer();
}
template<typename T>
decltype(auto) get_lowest_layer(T&& t,
                                std::false_type) noexcept
{
  return t;
}

}

/**
 *  Determines whether or not a certain type has a
 *  nested type alias `lowest_layer_type`.
 *
 *  If `T` has a nested type alias `lowest_layer_type`
 *  then an instantiation of this class template derives
 *  from `std::bool_constant<true>`, otherwise it derives
 *  from `std::bood_constant<false>`.
 *
 *  \tparam T
 *    The stream type.
 */
template<typename T>
class has_lowest_layer
#ifndef MCPP_DOXYGEN_RUNNING
: public detail::has_lowest_layer<T>
#endif
{};

/**
 *  A template variable for `has_lowest_layer<T>::value`.
 *
 *  \tparam T
 *    The template parameter to pass through to
 *    \ref has_lowest_layer.
 */
template<typename T>
constexpr bool has_lowest_layer_v = has_lowest_layer<T>::value;

/**
 *  A metafunction which computes the type of the
 *  lowest layer associated with a stream.
 *
 *  If `T` has a nested type alias `lowest_layer_type`
 *  then this metafunction has a nested type alias
 *  `type` which is `T::lowest_layer_type`, otherwise
 *  it has a nested type alias `type` which is `T`.
 *
 *  \tparam T
 *    The stream type.
 */
template<typename T>
class lowest_layer
#ifndef MCPP_DOXYGEN_RUNNING
: public detail::lowest_layer<T>
#endif
{};

/**
 *  A type alias for `lowest_layer<T>::type`.
 *
 *  \tparam T
 *    The template parameter to pass through to
 *    \ref lowest_layer.
 */
template<typename T>
using lowest_layer_t = typename lowest_layer<T>::type;

/**
 *  Retrieves the lowest layer associated with a certain
 *  stream.
 *
 *  If the stream type has a nested type alias
 *  `lowest_layer_type` then the lowest layer is obtained
 *  by calling the nullary member function `lowest_layer`
 *  thereupon, otherwise the stream itself is assumed
 *  to be the lowest layer and is simply returned
 *  untouched.
 *
 *  \tparam T
 *    The stream type (possibly ref-qualified).
 *
 *  \param [in] t
 *    The stream.
 *
 *  \return
 *    The lowest layer.
 */
template<typename T>
decltype(auto) get_lowest_layer(T&& t) noexcept {
  typename has_lowest_layer<std::decay_t<T>>::type tag;
  return detail::get_lowest_layer(t,
                                  tag);
}

}
