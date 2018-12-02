/**
 *  \file
 */

#pragma once

#include <system_error>
#include <boost/system/error_code.hpp>

namespace mcpp {

/**
 *  Obtains a `std::error_code` which is equivalent to
 *  a `boost::system::error_code`.
 *
 *  \param [in] ec
 *    The `boost::system::error_code`.
 *
 *  \return
 *    A `std::error_code` which is equivalent to `ec`.
 */
std::error_code to_error_code(boost::system::error_code ec) noexcept;
/**
 *  Returns its argument untouched.
 *
 *  This overload is provided to allow `mcpp::to_error_code`
 *  to be used in generic contexts to coalesce both
 *  `std::error_code` and `boost::system::error_code` to
 *  `std::error_code`.
 *
 *  \param [in] ec
 *    The `std::error_code`.
 *
 *  \return
 *    `ec`
 */
std::error_code to_error_code(std::error_code ec) noexcept;

/**
 *  Determines whether a certain `std::error_code`
 *  represents an end of file error.
 *
 *  \param [in] ec
 *    The `std::error_code` to check.
 *
 *  \return
 *    `true` if `ec` represents an end of file error,
 *    `false` otherwise.
 */
bool is_eof(std::error_code ec) noexcept;
/**
 *  Determines whether a certain `boost::system::error_code`
 *  represents an end of file error.
 *
 *  \param [in] ec
 *    The `boost::system::error_code` to check.
 *
 *  \return
 *    `true` if `ec` represents an end of file error,
 *    `false` otherwise.
 */
bool is_eof(boost::system::error_code ec) noexcept;

}
