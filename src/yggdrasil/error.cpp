#include <mcpp/yggdrasil/error.hpp>

#include <cassert>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <mcpp/rapidjson/string_parser.hpp>

namespace mcpp::yggdrasil {

namespace {

enum class errc {
  success,
  missing_error,
  missing_error_message,
  duplicate_error,
  duplicate_error_message,
  duplicate_cause
};

std::error_code make_error_code(errc code) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Yggdrasil/Error";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<errc>(code)) {
      case errc::success:
        return "Success";
      case errc::missing_error:
        return "No \"error\" key in Yggdrasil error message";
      case errc::missing_error_message:
        return "No \"errorMessage\" key in Yggdrasil error message";
      case errc::duplicate_error:
        return "Duplicate \"error\" key in Yggdrasil error message";
      case errc::duplicate_error_message:
        return "Duplicate \"errorMessage\" key in Yggdrasil error message";
      case errc::duplicate_cause:
        return "Duplicate \"cause\" key in Yggdrasil error message";
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
  return std::error_code(static_cast<int>(code),
                         category);
}

}

error_parser::error_parser(error& err) noexcept
  : obj_          (&err),
    begin_        (false),
    end_          (false),
    error_        (false),
    error_message_(false)
{
  clear();
}

void error_parser::clear() noexcept {
  assert(obj_);
  base::clear();
  obj_->error.clear();
  obj_->error_message.clear();
  obj_->cause.reset();
  begin_ = false;
  end_ = false;
  error_ = false;
  error_message_ = false;
}

bool error_parser::StartObject() noexcept {
  if (begin_) {
    return base::StartObject();
  }
  assert(!obj_->cause);
  assert(!end_);
  assert(!error_);
  assert(!error_message_);
  begin_ = true;
  return true;
}

bool error_parser::EndObject(std::size_t s) noexcept {
  if (!base::done() || end_) {
    return base::EndObject(s);
  }
  end_ = true;
  if (!error_) {
    error_code(make_error_code(errc::missing_error));
    return false;
  }
  if (!error_message_) {
    error_code(make_error_code(errc::missing_error_message));
    return false;
  }
  return true;
}

bool error_parser::Key(const char* str,
                       std::size_t size,
                       bool copy)
{
  assert(begin_);
  assert(!end_);
  assert(obj_);
  if (!base::done()) {
    return base::Key(str,
                     size,
                     copy);
  }
  std::string_view sv(str,
                      size);
  if (sv == "error") {
    if (error_) {
      error_code(make_error_code(errc::duplicate_error));
      return false;
    }
    error_ = true;
    base::emplace<rapidjson::string_parser>(obj_->error);
    return true;
  }
  if (sv == "errorMessage") {
    if (error_message_) {
      error_code(make_error_code(errc::duplicate_error_message));
      return false;
    }
    error_message_ = true;
    base::emplace<rapidjson::string_parser>(obj_->error_message);
    return true;
  }
  if (sv == "cause") {
    if (obj_->cause) {
      error_code(make_error_code(errc::duplicate_cause));
      return false;
    }
    obj_->cause.emplace();
    base::emplace<rapidjson::string_parser>(*obj_->cause);
    return true;
  }
  return base::Key(str,
                   size,
                   copy);
}

}
