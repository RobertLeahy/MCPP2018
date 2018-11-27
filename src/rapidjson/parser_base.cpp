#include <mcpp/rapidjson/parser_base.hpp>

#include <string>
#include <system_error>

namespace mcpp::rapidjson {

namespace {

enum class errc {
  success = 0,
  unexpected_type,
  unexpected_key
};

std::error_code make_error_code(errc err = errc::unexpected_type) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/RapidJSON/Parser Base";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<errc>(code)) {
      case errc::success:
        return "Success";
      case errc::unexpected_type:
        return "Unexpected JSON type in input";
      case errc::unexpected_key:
        return "Unexpected JSON key in input";
      default:
        break;
      }
      return "Unknown";
    }
    virtual std::error_condition default_error_condition(int code) const noexcept override {
      if (!code) {
        return std::error_condition();
      }
      return make_error_code(std::errc::bad_message).default_error_condition();
    }
  } category;
  return std::error_code(static_cast<int>(err),
                         category);
}

}

bool parser_base::Null() noexcept {
  ec_ = make_error_code();
  return false;
}

bool parser_base::Bool(bool) noexcept {
  ec_ = make_error_code();
  return false;
}

bool parser_base::Int(int) noexcept {
  ec_ = make_error_code();
  return false;
}

bool parser_base::Uint(unsigned) noexcept {
  ec_ = make_error_code();
  return false;
}

bool parser_base::Int64(std::int64_t) noexcept {
  ec_ = make_error_code();
  return false;
}

bool parser_base::Uint64(std::uint64_t) noexcept {
  ec_ = make_error_code();
  return false;
}

bool parser_base::Double(double) noexcept {
  ec_ = make_error_code();
  return false;
}

bool parser_base::String(const char*,
                         std::size_t,
                         bool) noexcept
{
  ec_ = make_error_code();
  return false;
}

bool parser_base::StartObject() noexcept {
  ec_ = make_error_code();
  return false;
}

bool parser_base::Key(const char*,
                      std::size_t,
                      bool) noexcept
{
  ec_ = make_error_code(errc::unexpected_key);
  return false;
}

bool parser_base::EndObject(std::size_t) noexcept {
  ec_ = make_error_code();
  return false;
}

bool parser_base::StartArray() noexcept {
  ec_ = make_error_code();
  return false;
}

bool parser_base::EndArray(std::size_t) noexcept {
  ec_ = make_error_code();
  return false;
}

bool parser_base::RawNumber(const char*,
                            std::size_t,
                            bool) noexcept
{
  ec_ = make_error_code();
  return false;
}

std::error_code parser_base::error_code() const noexcept {
  return ec_;
}

void parser_base::clear() noexcept {
  ec_.clear();
}

void parser_base::error_code(std::error_code ec) noexcept {
  ec_ = ec;
}

}
