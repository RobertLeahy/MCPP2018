#include <mcpp/yggdrasil/property.hpp>

#include <cassert>
#include <string>
#include <string_view>
#include <system_error>
#include <mcpp/rapidjson/string_parser.hpp>

namespace mcpp::yggdrasil {

namespace {

enum class errc {
  success,
  missing_name,
  missing_value,
  duplicate_name,
  duplicate_value
};

std::error_code make_error_code(errc code) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Yggdrasil/Property";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<errc>(code)) {
      case errc::success:
        return "Success";
      case errc::missing_name:
        return "No \"name\" key in Yggdrasil property object";
      case errc::missing_value:
        return "No \"value\" key in Yggdrasil property object";
      case errc::duplicate_name:
        return "Duplicate \"name\" key in Yggdrasil property object";
      case errc::duplicate_value:
        return "Duplicate \"value\" key in Yggdrasil property object";
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

property_parser::property_parser(property& p) noexcept
  : obj_         (&p),
    begin_       (false),
    end_         (false),
    name_        (false),
    value_       (false)
{}

void property_parser::clear() noexcept {
  assert(obj_);
  base::clear();
  begin_ = false;
  end_ = false;
  name_ = false;
  value_ = false;
}

bool property_parser::done() const noexcept {
  return end_;
}

bool property_parser::StartObject() noexcept {
  if (begin_) {
    return base::StartObject();
  }
  begin_ = true;
  return true;
}

bool property_parser::EndObject(std::size_t size) noexcept {
  if (!base::done() || end_) {
    return base::EndObject(size);
  }
  end_ = true;
  if (!name_) {
    error_code(make_error_code(errc::missing_name));
    return false;
  }
  if (!value_) {
    error_code(make_error_code(errc::missing_value));
    return false;
  }
  return true;
}

bool property_parser::Key(const char* str,
                          std::size_t size,
                          bool copy)
{
  assert(obj_);
  if (!begin_ ||
      end_    ||
      !base::done())
  {
    return base::Key(str,
                     size,
                     copy);
  }
  std::string_view sv(str,
                      size);
  if (sv == "name") {
    if (name_) {
      error_code(make_error_code(errc::duplicate_name));
      return false;
    }
    emplace<rapidjson::string_parser>(obj_->name);
    name_ = true;
    return true;
  }
  if (sv == "value") {
    if (value_) {
      error_code(make_error_code(errc::duplicate_value));
      return false;
    }
    emplace<rapidjson::string_parser>(obj_->value);
    value_ = true;
    return true;
  }
  return base::Key(str,
                   size,
                   copy);
}

}
