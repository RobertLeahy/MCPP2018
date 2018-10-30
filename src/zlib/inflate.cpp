#include <mcpp/zlib/inflate.hpp>

#include <string>
#include <system_error>

namespace mcpp::zlib {

namespace detail {

std::error_code make_error_code(inflate_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/ZLIB/Inflate";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<inflate_error>(code)) {
      case inflate_error::success:
        return "Success";
      case inflate_error::max_size:
        return "Maximum size of buffer reached";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<inflate_error>(code)) {
      case inflate_error::success:
        return std::error_condition();
      case inflate_error::max_size:
        return make_error_code(std::errc::not_enough_memory).default_error_condition();
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
