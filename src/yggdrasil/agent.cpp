#include <mcpp/yggdrasil/agent.hpp>

#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <mcpp/rapidjson/string_parser.hpp>
#include <mcpp/rapidjson/uint_parser.hpp>

namespace mcpp::yggdrasil {

namespace {

enum class errc {
  success,
  missing_name,
  missing_version,
  duplicate_name,
  duplicate_version
};

std::error_code make_error_code(errc code) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Yggdrasil/Agent";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<errc>(code)) {
      case errc::success:
        return "Success";
      case errc::missing_name:
        return "No \"name\" key in Yggdrasil agent object";
      case errc::missing_version:
        return "No \"version\" key in Yggdrasil agent object";
      case errc::duplicate_name:
        return "Duplicate \"name\" key in Yggdrasil agent object";
      case errc::duplicate_version:
        return "Duplicate \"version\" key in Yggdrasil agent object";
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

agent_parser::agent_parser(agent& a) noexcept
  : obj_    (&a), 
    begin_  (false),
    end_    (false),
    name_   (false),
    version_(false)
{}

void agent_parser::clear() noexcept {
  base::clear();
  begin_ = false;
  end_ = false;
  name_ = false;
  version_ = false;
}

bool agent_parser::done() const noexcept {
  return end_;
}

bool agent_parser::StartObject() noexcept {
  if (begin_) {
    return base::StartObject();
  }
  begin_ = true;
  return true;
}

bool agent_parser::EndObject(std::size_t s) noexcept {
  if (!base::done()) {
    return base::EndObject(s);
  }
  if (end_) {
    return base::EndObject(s);
  }
  if (!name_) {
    error_code(make_error_code(errc::missing_name));
    return false;
  }
  if (!version_) {
    error_code(make_error_code(errc::missing_version));
    return false;
  }
  end_ = true;
  return true;
}

bool agent_parser::Key(const char* str,
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
  if (sv == "name") {
    if (name_) {
      error_code(make_error_code(errc::duplicate_name));
      return false;
    }
    name_ = true;
    base::emplace<rapidjson::string_parser>(obj_->name);
    return true;
  }
  if (sv == "version") {
    if (version_) {
      error_code(make_error_code(errc::duplicate_version));
      return false;
    }
    version_ = true;
    base::emplace<rapidjson::uint_parser>(obj_->version);
    return true;
  }
  return base::Key(str,
                   size,
                   copy);
}

}
