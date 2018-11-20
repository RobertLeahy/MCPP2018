#include <mcpp/system_error.hpp>

#include <system_error>
#include <boost/asio/error.hpp>

namespace mcpp {

std::error_code to_error_code(boost::system::error_code ec) noexcept {
  return std::error_code(ec.value(),
                         ec.category());
}

std::error_code to_error_code(std::error_code ec) noexcept {
  return ec;
}

bool is_eof(std::error_code ec) noexcept {
  return ec.default_error_condition() == to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition();
}

bool is_eof(boost::system::error_code ec) noexcept {
  return ec.default_error_condition() == make_error_code(boost::asio::error::eof).default_error_condition();
}

}
