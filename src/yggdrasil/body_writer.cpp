#include <mcpp/yggdrasil/body_writer.hpp>

#include <string>
#include <system_error>

namespace mcpp::yggdrasil {

namespace detail {

std::error_code make_error_code(body_writer_error err) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Yggdrasil/Body Writer";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<body_writer_error>(code)) {
      case body_writer_error::success:
        return "Success";
      case body_writer_error::rapidjson_failed:
        //  TODO: Update message
        return "RapidJSON reported a failure (i.e. returned false from ::rapidjson::Reader::Parse)";
      default:
        break;
      }
      return "Success";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      switch (static_cast<body_writer_error>(code)) {
      case body_writer_error::success:
        return std::error_condition();
      case body_writer_error::rapidjson_failed:
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
