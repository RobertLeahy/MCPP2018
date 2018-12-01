#include <mcpp/yggdrasil/authenticate.hpp>

#include <cassert>
#include <string>
#include <string_view>
#include <system_error>
#include <mcpp/rapidjson/bool_parser.hpp>
#include <mcpp/rapidjson/string_parser.hpp>
#include <mcpp/yggdrasil/agent.hpp>
#include <mcpp/yggdrasil/profile.hpp>
#include <mcpp/yggdrasil/user.hpp>

namespace mcpp::yggdrasil {

namespace {

enum class errc {
  success,
  missing_agent,
  missing_username,
  missing_password,
  duplicate_agent,
  duplicate_username,
  duplicate_password,
  duplicate_client_token,
  duplicate_request_user,
  missing_access_token,
  missing_client_token,
  duplicate_access_token,
  duplicate_client_token_response,
  duplicate_available_profiles,
  duplicate_selected_profile,
  duplicate_user
};

std::error_code make_error_code(errc code) noexcept {
  static const class : public std::error_category {
  public:
    virtual const char* name() const noexcept override {
      return "MCPP/Yggdrasil/Authenticate";
    }
    virtual std::string message(int code) const override {
      switch (static_cast<errc>(code)) {
      case errc::success:
        return "Success";
      case errc::missing_agent:
        return "No \"agent\" key in Yggdrasil authenticate request object";
      case errc::missing_username:
        return "No \"username\" key in Yggdrasil authenticate request object";
      case errc::missing_password:
        return "No \"password\" key in Yggdrasil authenticate request object";
      case errc::duplicate_agent:
        return "Duplicate \"agent\" key in Yggdrasil authenticate request object";
      case errc::duplicate_username:
        return "Duplicate \"username\" key in Yggdrasil authenticate request object";
      case errc::duplicate_password:
        return "Duplicate \"password\" key in Yggdrasil authenticate request object";
      case errc::duplicate_client_token:
        return "Duplicate \"clientToken\" key in Yggdrasil authenticate request object";
      case errc::duplicate_request_user:
        return "Duplicate \"requestUser\" key in Yggdrasil authenticate request object";
      case errc::missing_access_token:
        return "No \"accessToken\" key in Yggdrasil authenticate response object";
      case errc::missing_client_token:
        return "No \"clientToken\" key in Yggdrasil authenticate response object";
      case errc::duplicate_access_token:
        return "Duplicate \"accessToken\" key in Yggdrasil authenticate response object";
      case errc::duplicate_client_token_response:
        return "Duplicate \"clientToken\" key in Yggdrasil authenticate response object";
      case errc::duplicate_available_profiles:
        return "Duplicate \"availableProfiles\" key in Yggdrasil authenticate response object";
      case errc::duplicate_selected_profile:
        return "Duplicate \"selectedProfile\" key in Yggdrasil authenticate response object";
      case errc::duplicate_user:
        return "Duplicate \"user\" key in Yggdrasil authenticate response object";
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

authenticate_request_parser::authenticate_request_parser(authenticate_request& a) noexcept
  : obj_         (&a),
    begin_       (false),
    end_         (false),
    agent_       (false),
    username_    (false),
    password_    (false),
    request_user_(false)
{}

void authenticate_request_parser::clear() noexcept {
  assert(obj_);
  base::clear();
  //  TODO: Do this in a way that allows
  //  memory re-use
  *obj_ = authenticate_request();
  begin_ = false;
  end_ = false;
  agent_ = false;
  username_ = false;
  password_ = false;
  request_user_ = false;
}

bool authenticate_request_parser::done() const noexcept {
  return end_;
}

bool authenticate_request_parser::StartObject() noexcept {
  if (begin_) {
    return base::StartObject();
  }
  begin_ = true;
  return true;
}

bool authenticate_request_parser::EndObject(std::size_t size) noexcept {
  if (!base::done() || end_) {
    return base::EndObject(size);
  }
  end_ = true;
  if (!agent_) {
    error_code(make_error_code(errc::missing_agent));
    return false;
  }
  if (!username_) {
    error_code(make_error_code(errc::missing_username));
    return false;
  }
  if (!password_) {
    error_code(make_error_code(errc::missing_password));
    return false;
  }
  return true;
}

bool authenticate_request_parser::Key(const char* str,
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
  if (sv == "agent") {
    if (agent_) {
      error_code(make_error_code(errc::duplicate_agent));
      return false;
    }
    agent_ = true;
    emplace<agent_parser>(obj_->agent);
    return true;
  }
  if (sv == "username") {
    if (username_) {
      error_code(make_error_code(errc::duplicate_username));
      return false;
    }
    emplace<rapidjson::string_parser>(obj_->username);
    username_ = true;
    return true;
  }
  if (sv == "password") {
    if (password_) {
      error_code(make_error_code(errc::duplicate_password));
      return false;
    }
    emplace<rapidjson::string_parser>(obj_->password);
    password_ = true;
    return true;
  }
  if (sv == "clientToken") {
    if (obj_->client_token) {
      error_code(make_error_code(errc::duplicate_client_token));
      return false;
    }
    obj_->client_token.emplace();
    emplace<rapidjson::string_parser>(*obj_->client_token);
    return true;
  }
  if (sv == "requestUser") {
    if (request_user_) {
      error_code(make_error_code(errc::duplicate_request_user));
      return false;
    }
    request_user_ = true;
    emplace<rapidjson::bool_parser>(obj_->request_user);
    return true;
  }
  return base::Key(str,
                   size,
                   copy);
}

authenticate_response_parser::authenticate_response_parser(authenticate_response& a) noexcept
  : obj_         (&a),
    begin_       (false),
    end_         (false),
    access_token_(false),
    client_token_(false)
{}

void authenticate_response_parser::clear() noexcept {
  assert(obj_);
  base::clear();
  //  TODO: Do this in a way that allows
  //  memory re-use
  *obj_ = authenticate_response();
  begin_ = false;
  end_ = false;
  access_token_ = false;
  client_token_ = false;
}

bool authenticate_response_parser::done() const noexcept {
  return end_;
}

bool authenticate_response_parser::StartObject() noexcept {
  if (begin_) {
    return base::StartObject();
  }
  begin_ = true;
  return true;
}

bool authenticate_response_parser::EndObject(std::size_t size) noexcept {
  if (!base::done() || end_) {
    return base::EndObject(size);
  }
  end_ = true;
  if (!access_token_) {
    error_code(make_error_code(errc::missing_access_token));
    return false;
  }
  if (!client_token_) {
    error_code(make_error_code(errc::missing_client_token));
    return false;
  }
  return true;
}

bool authenticate_response_parser::Key(const char* str,
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
  if (sv == "accessToken") {
    if (access_token_) {
      error_code(make_error_code(errc::duplicate_access_token));
      return false;
    }
    emplace<rapidjson::string_parser>(obj_->access_token);
    access_token_ = true;
    return true;
  }
  if (sv == "clientToken") {
    if (client_token_) {
      error_code(make_error_code(errc::duplicate_client_token_response));
      return false;
    }
    emplace<rapidjson::string_parser>(obj_->client_token);
    client_token_ = true;
    return true;
  }
  if (sv == "availableProfiles") {
    if (obj_->available_profiles) {
      error_code(make_error_code(errc::duplicate_available_profiles));
      return false;
    }
    obj_->available_profiles.emplace();
    emplace<rapidjson::array_parser<profile,
                                    profile_parser>>(*obj_->available_profiles);
    return true;
  }
  if (sv == "selectedProfile") {
    if (obj_->selected_profile) {
      error_code(make_error_code(errc::duplicate_selected_profile));
      return false;
    }
    obj_->selected_profile.emplace();
    emplace<profile_parser>(*obj_->selected_profile);
    return true;
  }
  if (sv == "user") {
    if (obj_->user) {
      error_code(make_error_code(errc::duplicate_user));
      return false;
    }
    obj_->user.emplace();
    emplace<user_parser>(*obj_->user);
    return true;
  }
  return base::Key(str,
                   size,
                   copy);
}

}
