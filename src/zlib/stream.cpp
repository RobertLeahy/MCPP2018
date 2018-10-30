#include <mcpp/zlib/stream.hpp>

#include <string>
#include <system_error>

namespace mcpp::zlib {

namespace detail {

std::error_code make_error_code(stream_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/ZLIB/Stream";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<stream_error>(code)) {
      case stream_error::success:
        return "Success";
      case stream_error::avail_in_overflow:
        return "Overflow calculating input buffer size";
      case stream_error::avail_out_overflow:
        return "Overflow calculating output buffer size";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<stream_error>(code)) {
      case stream_error::success:
        return std::error_condition();
      case stream_error::avail_in_overflow:
      case stream_error::avail_out_overflow:
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
