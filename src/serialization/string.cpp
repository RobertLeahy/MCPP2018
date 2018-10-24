#include <mcpp/serialization/string.hpp>

#include <string>
#include <system_error>
#include <mcpp/system_error.hpp>

namespace mcpp::serialization {

namespace detail {

std::error_code make_error_code(string_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Serialization/String";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<string_error>(code)) {
      case string_error::success:
        return "Success";
      case string_error::negative_size:
        return "String has negative size prefix";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<string_error>(code)) {
      case string_error::success:
        return std::error_condition();
      case string_error::negative_size:
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
