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

}
