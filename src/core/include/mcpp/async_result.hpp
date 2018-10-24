/**
 *  \file
 */

#pragma once

#include <type_traits>
#include <boost/asio/async_result.hpp>

namespace mcpp {

/**
 *  A type alias for an appropriate instantiation
 *  of `boost::asio::async_result`.
 *
 *  \tparam DeducedCompletionToken
 *    A `CompletionToken` possibly with cv- and/or
 *    ref-qualification added (i.e. a `CompletionToken`
 *    as deduced as a template parameter of an initiating
 *    function).
 *  \tparam Signature
 *    A function signature.
 */
template<typename DeducedCompletionToken,
         typename Signature>
using async_result_t = boost::asio::async_result<std::decay_t<DeducedCompletionToken>,
                                                 Signature>;

/**
 *  A type alias for the completion handler type
 *  associated with a certain `CompletionToken` and
 *  `Signature` as deduced via `boost::asio::async_result`.
 *
 *  \tparam DeducedCompletionToken
 *    A `CompletionToken` possibly with cv- and/or
 *    ref-qualification added (i.e. a `CompletionToken`
 *    as deduced as a template parameter of an initiating
 *    function).
 *  \tparam Signature
 *    A function signature.
 */
template<typename DeducedCompletionToken,
         typename Signature>
using completion_handler_t = typename async_result_t<DeducedCompletionToken,
                                                     Signature>::completion_handler_type;

}
