/**
 *  \file
 */

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <mcpp/rapidjson/key.hpp>
#include <mcpp/rapidjson/state_machine_parser_base.hpp>
#include <mcpp/rapidjson/string.hpp>
#include <mcpp/rapidjson/string_parser.hpp>

namespace mcpp::yggdrasil {

/**
 *  Represents an error from the Yggdrasil
 *  HTTP API.
 */
class error {
public:
  /**
   *  A short description of the error.
   */
  std::string error;
  /**
   *  A longer description of the error suitable
   *  for presentation to a user.
   */
  std::string error_message;
  /**
   *  The underlying cause of the error, if any.
   */
  std::optional<std::string> cause;
};

/**
 *  Converts a \ref error object to JSON by
 *  sending a sequence of SAX events to a
 *  RapidJSON `Writer`.
 *
 *  \tparam Writer
 *    A type which satisfies the requirements of
 *    a `Writer` (from RapidJSON).
 *
 *  \param [in] obj
 *    The \ref error to convert to JSON.
 *  \param [in, out] writer
 *    The object to which SAX events shall be
 *    sent.
 *
 *  \return
 *    `true` if all the calls to `writer` returned
 *    `true`, `false` otherwise (note that after the
 *    first call returns `false` no further calls
 *    shall be made).
 */
template<typename Writer>
bool to_json(const error& obj,
             Writer& writer)
{
  return writer.StartObject()                 &&
         rapidjson::key("error",
                        writer)               &&
         rapidjson::string(obj.error,
                           writer)            &&
         rapidjson::key("errorMessage",
                        writer)               &&
         rapidjson::string(obj.error_message,
                           writer)            &&
         (!obj.cause ||
          (rapidjson::key("cause",
                          writer) &&
           rapidjson::string(*obj.cause,
                             writer)))        &&
         writer.EndObject();
}

/**
 *  An object which parses \ref error objects
 *  from a sequence of RapidJSON SAX events.
 */
class error_parser
#ifndef MCPP_DOXYGEN_RUNNING
: public rapidjson::state_machine_parser_base<rapidjson::string_parser>
#endif
{
private:
  using base = rapidjson::state_machine_parser_base<rapidjson::string_parser>;
public:
  /**
   *  Creates a parser which parses into a certain
   *  \ref error object.
   *
   *  \param [in] err
   *    A reference to the \ref error "object" into
   *    which to parse. Reference must be valid until
   *    after the newly-constructed object receives the
   *    last SAX event.
   */
  explicit error_parser(error& err) noexcept;
#ifndef MCPP_DOXYGEN_RUNNING
  void clear() noexcept;
  void done() const noexcept;
  bool StartObject() noexcept;
  bool EndObject(std::size_t) noexcept;
  bool Key(const char*,
           std::size_t,
           bool);
#endif
private:
  error*       obj_;
  bool         begin_;
  bool         end_;
  bool         error_;
  bool         error_message_;
};

}
