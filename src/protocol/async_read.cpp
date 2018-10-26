#include <mcpp/protocol/async_read.hpp>

#include <string>
#include <system_error>

namespace mcpp::protocol {

namespace detail {

std::error_code make_error_code(limit_after_read_length_initiating_function_error err) noexcept {
  using error = limit_after_read_length_initiating_function_error;
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Protocol/AsyncRead/Limit";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<error>(code)) {
      case error::success:
        return "Success";
      case error::too_long:
        return "Length of Minecraft protocol packet longer than maximum";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<error>(code)) {
      case error::success:
        return std::error_condition();
      case error::too_long:
        return make_error_code(std::errc::result_out_of_range).default_error_condition();
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
