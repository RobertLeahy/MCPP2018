/**
 *  \file
 */

#pragma once

#include <system_error>

namespace mcpp::zlib {

/**
 *  Creates a `std::error_code` from an `int`
 *  error code returned by the zlib C API.
 *
 *  \param [in] code
 *    A zlib error code.
 *
 *  \return
 *    A `std::error_code` representing the same
 *    error.
 */
std::error_code make_error_code(int code) noexcept;

}
