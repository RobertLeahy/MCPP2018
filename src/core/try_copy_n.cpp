#include <mcpp/try_copy_n.hpp>

#include <string>
#include <system_error>
#include <boost/asio/error.hpp>
#include <mcpp/system_error.hpp>

namespace mcpp {

namespace detail {

std::error_code make_error_code(try_copy_n_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Core/TryCopyN";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<try_copy_n_error>(code)) {
      case try_copy_n_error::success:
        return "Success";
      case try_copy_n_error::eof:
        return "Unexpected end of input";
      case try_copy_n_error::negative:
        return "Number to try and copy is negative";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<try_copy_n_error>(code)) {
      case try_copy_n_error::success:
        return std::error_condition();
      case try_copy_n_error::eof:
        return to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition();
      case try_copy_n_error::negative:
        return make_error_code(std::errc::argument_out_of_domain).default_error_condition();
      default:
        break;
      }
      return std::error_condition(code,
                                  *this);
    }
  } category;
  return std::error_code(static_cast<int>(err),
                         category);
}

}

}
