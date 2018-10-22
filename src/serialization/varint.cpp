#include <mcpp/serialization/varint.hpp>

#include <string>
#include <system_error>
#include <boost/asio/error.hpp>
#include <mcpp/system_error.hpp>

namespace mcpp::serialization {

namespace detail {

std::error_code make_error_code(varint_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Serialization/Varint";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<varint_error>(code)) {
      case varint_error::success:
        return "Success";
      case varint_error::max:
        return "Continuation on last possible byte";
      case varint_error::overlong:
        return "Overlong varint representation";
      case varint_error::overflow:
        return "Representation encodes number too large for target integer type";
      case varint_error::eof:
        return "End of buffer while parsing varint";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<varint_error>(code)) {
      case varint_error::success:
        return std::error_condition();
      case varint_error::max:
      case varint_error::overflow:
        return make_error_code(std::errc::value_too_large).default_error_condition();
      case varint_error::overlong:
        return make_error_code(std::errc::bad_message).default_error_condition();
      case varint_error::eof:
        return to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition();
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
