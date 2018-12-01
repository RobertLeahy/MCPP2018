/**
 *  \file
 */

#pragma once

#include <cstddef>
#include <string>
#include <mcpp/rapidjson/bool_parser.hpp>
#include <mcpp/rapidjson/key.hpp>
#include <mcpp/rapidjson/state_machine_parser_base.hpp>
#include <mcpp/rapidjson/string.hpp>
#include <mcpp/rapidjson/string_parser.hpp>

namespace mcpp::yggdrasil {

/**
 *  Represents a profile object used in the
 *  Yggdrasil API.
 */
class profile {
public:
  std::string id;
  std::string name;
  bool        legacy = false;
};

/**
 *  Transforms a \ref profile object
 *  into a sequence of SAX events.
 *
 *  \tparam Writer
 *    A RapidJSON-compatible `Writer` type.
 *
 *  \param [in] obj
 *    The \ref profile object.
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
bool to_json(const profile& obj,
             Writer& writer)
{
  return writer.StartObject()      &&
         rapidjson::key("id",
                        writer)    &&
         rapidjson::string(obj.id,
                           writer) &&
         rapidjson::key("name",
                        writer)    &&
         rapidjson::string(obj.name,
                           writer) &&
         (!obj.legacy ||
          (rapidjson::key("legacy",
                          writer) &&
           writer.Bool(true)))     &&
         writer.EndObject();
}

/**
 *  Receives SAX events by being a RapidJSON `Reader`
 *  and transforms them into a \ref profile
 *  object if appropriate.
 */
class profile_parser
#ifndef MCPP_DOXYGEN_RUNNING
: public rapidjson::state_machine_parser_base<rapidjson::string_parser,
                                              rapidjson::bool_parser>
#endif
{
private:
  using base = rapidjson::state_machine_parser_base<rapidjson::string_parser,
                                                    rapidjson::bool_parser>;
public:
  /**
   *  Creates a new profile_parser which parses
   *  into a certain \ref profile object.
   *
   *  \param [in] p
   *    A reference to the \ref profile object into
   *    which to parse. Reference must remain valid until members
   *    of the newly-created object are no longer liable to be
   *    called or the behavior is undefined.
   */
  explicit profile_parser(profile& p) noexcept;
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
  profile* obj_;
  bool     begin_;
  bool     end_;
  bool     id_;
  bool     name_;
  bool     legacy_;
};

}
