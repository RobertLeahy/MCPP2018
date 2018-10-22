#include <mcpp/system_error.hpp>

#include <system_error>

namespace mcpp {

/**
 *  Obtains a `std::error_code` which encapsulates
 *  the same error (or lack thereof) as a
 *  `boost::system::error_code`.
 *
 *  \param [in] ec
 *    The `boost::system::error_code`.
 *
 *  \return
 *    A `std::error_code` equivalent to `ec`.
 */
std::error_code to_error_code(boost::system::error_code ec) noexcept {
  return std::error_code(ec.value(),
                         ec.category());
}

}
