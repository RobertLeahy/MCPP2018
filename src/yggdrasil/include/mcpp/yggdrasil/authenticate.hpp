/**
 *  \file
 */

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "agent.hpp"
#include <mcpp/rapidjson/array_parser.hpp>
#include <mcpp/rapidjson/bool_parser.hpp>
#include <mcpp/rapidjson/key.hpp>
#include <mcpp/rapidjson/state_machine_parser_base.hpp>
#include <mcpp/rapidjson/string.hpp>
#include <mcpp/rapidjson/string_parser.hpp>
#include "profile.hpp"
#include "user.hpp"

namespace mcpp::yggdrasil {

/**
 *  Represents a request against the Yggdrasil API
 *  to authenticate a user using their password.
 *
 *  \sa
 *    authenticate_response
 */
class authenticate_request {
public:
  yggdrasil::agent           agent;
  std::string                username;
  std::string                password;
  std::optional<std::string> client_token;
  bool                       request_user = false;
};

/**
 *  Transforms an \ref authenticate_request object
 *  into a sequence of SAX events.
 *
 *  \tparam Writer
 *    A RapidJSON-compatible `Writer` type.
 *
 *  \param [in] obj
 *    The \ref authenticate_request object.
 *  \param [in, out] writer
 *    A `Writer` object which shall receive the SAX events
 *    corresponding to `obj`.
 *
 *  \return
 *    `true` if all SAX event handlers returned `true`,
 *    `false` otherwise. Note that SAX events will not
 *    be dispatched after the first such handler returns
 *    `false` (the function instead returns immediately).
 */
template<typename Writer>
bool to_json(const authenticate_request& obj,
             Writer& writer)
{
  return writer.StartObject()          &&
         rapidjson::key("agent",
                        writer)        &&
         yggdrasil::to_json(obj.agent,
                            writer)    &&
         rapidjson::key("username",
                        writer)        &&
         rapidjson::string(obj.username,
                           writer)     &&
         rapidjson::key("password",
                        writer)        &&
         rapidjson::string(obj.password,
                           writer)     &&
         (!obj.client_token ||
          (rapidjson::key("clientToken",
                          writer) &&
           rapidjson::string(*obj.client_token,
                             writer))) &&
         (!obj.request_user ||
          (rapidjson::key("requestUser",
                          writer) &&
           writer.Bool(true)))         &&
         writer.EndObject();
}

/**
 *  Receives SAX events by being a RapidJSON `Reader`
 *  and transforms them into an \ref authenticate_request
 *  object if appropriate.
 */
class authenticate_request_parser
#ifndef MCPP_DOXYGEN_RUNNING
: public rapidjson::state_machine_parser_base<agent_parser,
                                              rapidjson::string_parser,
                                              rapidjson::bool_parser>
#endif
{
private:
  using base = rapidjson::state_machine_parser_base<agent_parser,
                                                    rapidjson::string_parser,
                                                    rapidjson::bool_parser>;
public:
  /**
   *  Creates a new authenticate_request_parser which parses
   *  into a certain \ref authenticate_request object.
   *
   *  \param [in] a
   *    A reference to the \ref authenticate_request object into
   *    which to parse. Reference must remain valid until members
   *    of this function are no longer liable to be called or
   *    the behavior is undefined.
   */
  explicit authenticate_request_parser(authenticate_request& a) noexcept;
#ifndef MCPP_DOXYGEN_RUNNING
  void clear() noexcept;
  bool done() const noexcept;
  bool StartObject() noexcept;
  bool EndObject(std::size_t) noexcept;
  bool Key(const char*,
           std::size_t,
           bool);
#endif
private:
  authenticate_request* obj_;
  bool                  begin_;
  bool                  end_;
  bool                  agent_;
  bool                  username_;
  bool                  password_;
  bool                  request_user_;
};

/**
 *  Represents a response to a \ref authenticate_request "request"
 *  to authenticate a user using their password.
 *
 *  \sa
 *    authenticate_request
 */
class authenticate_response {
public:
  using available_profiles_type = std::vector<profile>;
  std::string                            access_token;
  std::string                            client_token;
  std::optional<available_profiles_type> available_profiles;
  std::optional<profile>                 selected_profile;
  std::optional<yggdrasil::user>         user;
};

/**
 *  Transforms an \ref authenticate_response object
 *  into a sequence of SAX events.
 *
 *  \tparam Writer
 *    A RapidJSON-compatible `Writer` type.
 *
 *  \param [in] obj
 *    The \ref authenticate_response object.
 *  \param [in, out] writer
 *    A `Writer` object which shall receive the SAX events
 *    corresponding to `obj`.
 *
 *  \return
 *    `true` if all SAX event handlers returned `true`,
 *    `false` otherwise. Note that SAX events will not
 *    be dispatched after the first such handler returns
 *    `false` (the function instead returns immediately).
 */
template<typename Writer>
bool to_json(const authenticate_response& obj,
             Writer& writer)
{
  if (!(writer.StartObject()      &&
        rapidjson::key("accessToken",
                       writer)    &&
        rapidjson::string(obj.access_token,
                          writer) &&
        rapidjson::key("clientToken",
                       writer)    &&
        rapidjson::string(obj.client_token,
                          writer)))
  {
    return false;
  }
  if (obj.available_profiles) {
    if (!(rapidjson::key("availableProfiles",
                         writer) &&
          writer.StartArray()))
    {
      return false;
    }
    for (auto&& profile : *obj.available_profiles) {
      if (!yggdrasil::to_json(profile,
                              writer))
      {
        return false;
      }
    }
    if (!writer.EndArray()) {
      return false;
    }
  }
  return (!obj.selected_profile ||
          (rapidjson::key("selectedProfile",
                          writer) &&
           yggdrasil::to_json(*obj.selected_profile,
                              writer))) &&
         (!obj.user ||
          (rapidjson::key("user",
                          writer) &&
           yggdrasil::to_json(*obj.user,
                              writer))) &&
         writer.EndObject();
}

/**
 *  Receives SAX events by being a RapidJSON `Reader`
 *  and transforms them into an \ref authenticate_response
 *  object if appropriate.
 */
class authenticate_response_parser
#ifndef MCPP_DOXYGEN_RUNNING
: public rapidjson::state_machine_parser_base<rapidjson::array_parser<profile,
                                                                      profile_parser>,
                                              profile_parser,
                                              user_parser,
                                              rapidjson::string_parser,
                                              rapidjson::bool_parser>
#endif
{
private:
  using base = rapidjson::state_machine_parser_base<rapidjson::array_parser<profile,
                                                                            profile_parser>,
                                                    profile_parser,
                                                    user_parser,
                                                    rapidjson::string_parser,
                                                    rapidjson::bool_parser>;
public:
  /**
   *  Creates a new authenticate_response_parser which parses
   *  into a certain \ref authenticate_response object.
   *
   *  \param [in] a
   *    A reference to the \ref authenticate_response object into
   *    which to parse. Reference must remain valid until members
   *    of this function are no longer liable to be called or
   *    the behavior is undefined.
   */
  explicit authenticate_response_parser(authenticate_response& a) noexcept;
#ifndef MCPP_DOXYGEN_RUNNING
  void clear() noexcept;
  bool done() const noexcept;
  bool StartObject() noexcept;
  bool EndObject(std::size_t) noexcept;
  bool Key(const char*,
           std::size_t,
           bool);
#endif
private:
  authenticate_response* obj_;
  bool                   begin_;
  bool                   end_;
  bool                   access_token_;
  bool                   client_token_;
};

}
