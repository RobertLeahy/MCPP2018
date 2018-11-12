#include <mcpp/protocol/decompress.hpp>

#include <string>
#include <system_error>

namespace mcpp::protocol {

namespace detail {

std::error_code make_error_code(decompress_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Protocol/Decompress";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<decompress_error>(code)) {
      case decompress_error::success:
        return "Success";
      case decompress_error::length_negative:
        return "Negative length";
      case decompress_error::padded:
        return "Bytes remained after deflated body";
      case decompress_error::wrong_length:
        return "Length of decompressed data was incorrect";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<decompress_error>(code)) {
      case decompress_error::success:
        return std::error_condition();
      case decompress_error::length_negative:
        return make_error_code(std::errc::value_too_large).default_error_condition();
      case decompress_error::padded:
        //  TODO
      case decompress_error::wrong_length:
        //  TODO
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
