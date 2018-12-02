#include <mcpp/yggdrasil/body_reader.hpp>

#include <string>
#include <system_error>

namespace mcpp::yggdrasil {

namespace detail {

std::error_code make_error_code(body_reader_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Yggdrasil/Body Reader";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<body_reader_error>(code)) {
      case body_reader_error::success:
        return "Success";
      case body_reader_error::rapidjson_failed:
        return "RapidJSON reported a failure (i.e. returned false from ::rapidjson::Reader::Parse)";
      case body_reader_error::no_body:
        return "No HTTP body";
      default:
        break;
      }
      return "Success";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<body_reader_error>(code)) {
      case body_reader_error::success:
        return std::error_condition();
      case body_reader_error::no_body:
        return make_error_code(std::errc::bad_message).default_error_condition();
      case body_reader_error::rapidjson_failed:
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
