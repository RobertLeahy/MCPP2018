/**
 *  \file
 */

#pragma once

#include <system_error>

namespace mcpp::crypto {

/**
 *  Wraps an OpenSSL error code in a `std::error_code`
 *  object.
 *
 *  Note that due to the fact `std::error_code` contains
 *  an `int` and due to the fact OpenSSL uses `unsigned long`
 *  for its error codes it is possible for `e` not to be
 *  representable as a `std::error_code`. In this case
 *  `make_error_code(std::errc::value_too_large)` is returned.
 *
 *  \param [in] e
 *    The OpenSSL error code.
 *
 *  \return
 *    A `std::error_code` wrapping `e` or a different
 *    `std::error_code` as described.
 */
std::error_code make_error_code(unsigned long e) noexcept;

/**
 *  Calls `ERR_get_error` and uses \ref make_error_code
 *  to transfrom it to a `std::error_code` then calls
 *  `ERR_get_error` repeatedly to clear this thread's
 *  error code queue.
 *
 *  If the initial call to `ERR_get_error` returns `0`
 *  then a truthy `std::error_code` is returned.
 *
 *  \return
 *    A `std::error_code` as described.
 */
std::error_code get_error_code() noexcept;

}
