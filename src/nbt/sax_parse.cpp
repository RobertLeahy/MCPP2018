#include <mcpp/nbt/sax_parse.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <system_error>
#include <boost/asio/error.hpp>
#include <mcpp/system_error.hpp>

namespace mcpp::nbt {

namespace detail {

std::error_code make_error_code(sax_parse_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/NBT/SAX Parse";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<sax_parse_error>(code)) {
      case sax_parse_error::success:
        return "Success";
      case sax_parse_error::eof:
        return "Unexpected end of input";
      case sax_parse_error::first_tag_not_compound:
        return "NBT does not begin with TAG_Compound";
      case sax_parse_error::invalid_tag:
        return "Invalid NBT tag";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<sax_parse_error>(code)) {
      case sax_parse_error::success:
        return std::error_condition();
      case sax_parse_error::eof:
        return to_error_code(make_error_code(boost::asio::error::eof)).default_error_condition();
      case sax_parse_error::first_tag_not_compound:
      case sax_parse_error::invalid_tag:
        return make_error_code(std::errc::bad_message).default_error_condition();
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

std::error_code sax_parse_check_tag(std::byte tag) noexcept {
  switch (std::to_integer<std::uint8_t>(tag)) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
    return std::error_code();
  default:
    break;
  }
  return make_error_code(sax_parse_error::invalid_tag);
}

}

}
