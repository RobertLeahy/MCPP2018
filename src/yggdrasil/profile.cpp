#include <mcpp/yggdrasil/profile.hpp>

#include <cassert>
#include <string>
#include <string_view>
#include <system_error>
#include <mcpp/rapidjson/bool_parser.hpp>
#include <mcpp/rapidjson/string_parser.hpp>

namespace mcpp::yggdrasil {

namespace {

enum class errc {
  success,
  missing_id,
  missing_name,
  duplicate_id,
  duplicate_name,
  duplicate_legacy,
};

std::error_code make_error_code(errc code) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Yggdrasil/Profile";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<errc>(code)) {
      case errc::success:
        return "Success";
      case errc::missing_id:
        return "No \"id\" key in Yggdrasil profile object";
      case errc::missing_name:
        return "No \"name\" key in Yggdrasil profile object";
      case errc::duplicate_id:
        return "Duplicate \"id\" key in Yggdrasil profile object";
      case errc::duplicate_name:
        return "Duplicate \"name\" key in Yggdrasil profile object";
      case errc::duplicate_legacy:
        return "Duplicate \"legacy\" key in Yggdrasil profile object";
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

profile_parser::profile_parser(profile& p) noexcept
  : obj_         (&p),
    begin_       (false),
    end_         (false),
    id_          (false),
    name_        (false),
    legacy_      (false)
{}

void profile_parser::clear() noexcept {
  assert(obj_);
  base::clear();
  obj_->id.clear();
  obj_->name.clear();
  obj_->legacy = false;
  begin_ = false;
  end_ = false;
  id_ = false;
  name_ = false;
  legacy_ = false;
}

bool profile_parser::done() const noexcept {
  return end_;
}

bool profile_parser::StartObject() noexcept {
  if (begin_) {
    return base::StartObject();
  }
  begin_ = true;
  return true;
}

bool profile_parser::EndObject(std::size_t size) noexcept {
  if (!base::done() || end_) {
    return base::EndObject(size);
  }
  end_ = true;
  if (!id_) {
    error_code(make_error_code(errc::missing_id));
    return false;
  }
  if (!name_) {
    error_code(make_error_code(errc::missing_name));
    return false;
  }
  return true;
}

bool profile_parser::Key(const char* str,
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
    id_ = true;
    emplace<rapidjson::string_parser>(obj_->id);
    return true;
  }
  if (sv == "name") {
    if (name_) {
      error_code(make_error_code(errc::duplicate_name));
      return false;
    }
    emplace<rapidjson::string_parser>(obj_->name);
    name_ = true;
    return true;
  }
  if (sv == "legacy") {
    if (legacy_) {
      error_code(make_error_code(errc::duplicate_legacy));
      return false;
    }
    legacy_ = true;
    emplace<rapidjson::bool_parser>(obj_->legacy);
    return true;
  }
  return base::Key(str,
                   size,
                   copy);
}

}
