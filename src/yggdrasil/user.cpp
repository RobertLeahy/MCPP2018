#include <mcpp/yggdrasil/user.hpp>

#include <cassert>
#include <string>
#include <string_view>
#include <system_error>
#include <mcpp/rapidjson/array_parser.hpp>
#include <mcpp/rapidjson/string_parser.hpp>
#include <mcpp/yggdrasil/property.hpp>

namespace mcpp::yggdrasil {

namespace {

enum class errc {
  success,
  missing_id,
  missing_properties,
  duplicate_id,
  duplicate_properties
};

std::error_code make_error_code(errc code) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Yggdrasil/User";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<errc>(code)) {
      case errc::success:
        return "Success";
      case errc::missing_id:
        return "No \"id\" key in Yggdrasil user object";
      case errc::missing_properties:
        return "No \"properties\" key in Yggdrasil user object";
      case errc::duplicate_id:
        return "Duplicate \"id\" key in Yggdrasil user object";
      case errc::duplicate_properties:
        return "Duplicate \"properties\" key in Yggdrasil user object";
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

user_parser::user_parser(user& u) noexcept
  : obj_         (&u),
    begin_       (false),
    end_         (false),
    id_          (false),
    properties_  (false)
{}

void user_parser::clear() noexcept {
  assert(obj_);
  base::clear();
  obj_->properties.clear();
  begin_ = false;
  end_ = false;
  id_ = false;
  properties_ = false;
}

bool user_parser::done() const noexcept {
  return end_;
}

bool user_parser::StartObject() noexcept {
  if (begin_) {
    return base::StartObject();
  }
  begin_ = true;
  return true;
}

bool user_parser::EndObject(std::size_t size) noexcept {
  if (!base::done() || end_) {
    return base::EndObject(size);
  }
  end_ = true;
  if (!id_) {
    error_code(make_error_code(errc::missing_id));
    return false;
  }
  if (!properties_) {
    error_code(make_error_code(errc::missing_properties));
    return false;
  }
  return true;
}

bool user_parser::Key(const char* str,
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
  if (sv == "id") {
    if (id_) {
      error_code(make_error_code(errc::duplicate_id));
      return false;
    }
    emplace<rapidjson::string_parser>(obj_->id);
    id_ = true;
    return true;
  }
  if (sv == "properties") {
    if (properties_) {
      error_code(make_error_code(errc::duplicate_properties));
      return false;
    }
    emplace<rapidjson::array_parser<property,
                                    property_parser>>(obj_->properties);
    properties_ = true;
    return true;
  }
  return base::Key(str,
                   size,
                   copy);
}

}
