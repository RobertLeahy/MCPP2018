#include <mcpp/zlib/deflate.hpp>

#include <string>
#include <system_error>

namespace mcpp::zlib {

namespace detail {

std::error_code make_error_code(deflate_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/ZLIB/Deflate";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<deflate_error>(code)) {
      case deflate_error::success:
        return "Success";
      case deflate_error::bound_overflow:
        return "Overflow calculating output bound";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<deflate_error>(code)) {
      case deflate_error::success:
        return std::error_condition();
      case deflate_error::bound_overflow:
        return make_error_code(std::errc::value_too_large).default_error_condition();
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
