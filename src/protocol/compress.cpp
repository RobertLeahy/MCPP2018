#include <mcpp/protocol/compress.hpp>

#include <system_error>

namespace mcpp::protocol {

namespace detail {

std::error_code make_error_code(compress_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Protocol/Compress";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<compress_error>(code)) {
      case compress_error::success:
        return "Success";
      case compress_error::size_overflow:
        return "Overflow calculating uncompressed size";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<compress_error>(code)) {
      case compress_error::success:
        return std::error_condition();
      case compress_error::size_overflow:
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
