/**
 *  \file
 */

#pragma once

namespace mcpp {

/**
 *  Removes rvalue reference qualification from a type.
 *
 *  Instantiations of this class template contain a
 *  nested type alias `type` which is `T` with rvalue
 *  reference qualification removed. If `T` is not an
 *  rvalue reference type `type` and `T` shall alias
 *  the same type.
 *
 *  \tparam T
 *    The type from which to strip rvalue reference
 *    qualification.
 */
template<typename T>
class remove_rvalue_reference {
#ifndef MCPP_DOXYGEN_RUNNING
public:
  using type = T;
#endif
};
#ifndef MCPP_DOXYGEN_RUNNING
template<typename T>
class remove_rvalue_reference<T&&> {
public:
  using type = T;
};
#endif

/**
 *  A type alias for `remove_rvalue_reference<T>::type`.
 *
 *  \tparam T
 *    The template parameter to pass through to
 *    \ref remove_rvalue_reference.
 */
template<typename T>
using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;

}
