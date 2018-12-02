/** 
 *  \file
 */

#pragma once

#include <memory>
#include <type_traits>
#include "body_reader.hpp"
#include "body_writer.hpp"

namespace mcpp::yggdrasil {

/**
 *  A model of the `Body` concept which uses
 *  a \ref body_reader to parse the body.
 *
 *  \tparam isRequest
 *    `true` if a request should be parsed, `false`
 *    otherwise.
 *  \tparam T
 *    The message type.
 *  \tparam Parser
 *    The parser to use for `T`.
 */
template<bool isRequest,
         typename T,
         typename Parser>
class body {
public:
  /**
   *  Exposes the `isRequest` template parameter as
   *  a static member variable.
   */
  static constexpr bool is_request = isRequest;
  /**
   *  A type alias for the `Parser` template parameter.
   */
  using parser_type = Parser;
  /**
   *  A type alias for an appropriate instantiation of
   *  \ref body_reader.
   */
  using reader = body_reader<is_request,
                             T,
                             parser_type>;
  /**
   *  A type alias for an appropriate instantiation of
   *  \ref body_writer.
   */
  using writer = body_writer<is_request,
                             T>;
  /**
   *  A type alias for the actual type used to represent
   *  the body.
   *
   *  Note that this may not be `T` due to the fact when
   *  parsing responses the body may be either `T` or
   *  \ref error "an error" and therefore in this instance
   *  the body is a `std::variant` over these two types.
   */
  using value_type = typename reader::value_type;
  static_assert(std::is_same_v<value_type,
                               typename writer::value_type>);
};

/**
 *  A type alias for \ref body with the first
 *  template parameter set to `true`.
 */
template<typename T,
         typename Parser>
using request_body = body<true,
                          T,
                          Parser>;

/**
 *  A type alias for \ref body with the first
 *  template parameter set to `false`.
 */
template<typename T,
         typename Parser>
using response_body = body<false,
                           T,
                           Parser>;

}
