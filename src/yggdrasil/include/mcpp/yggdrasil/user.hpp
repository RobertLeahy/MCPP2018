/**
 *  \file
 */

#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <mcpp/rapidjson/array_parser.hpp>
#include <mcpp/rapidjson/key.hpp>
#include <mcpp/rapidjson/state_machine_parser_base.hpp>
#include <mcpp/rapidjson/string.hpp>
#include <mcpp/rapidjson/string_parser.hpp>
#include "property.hpp"

namespace mcpp::yggdrasil {

/**
 *  Represents a user object used in the
 *  Yggdrasil API.
 */
class user {
public:
  using properties_type = std::vector<property>;
  std::string     id;
  properties_type properties;
};

/**
 *  Transforms a \ref user object into a sequence
 *  of SAX events.
 *
 *  \tparam Writer
 *    A RapidJSON-compatible `Writer` type.
 *
 *  \param [in] obj
 *    The \ref user object.
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
bool to_json(const user& obj,
             Writer& writer)
{
  if (!(writer.StartObject()     &&
        rapidjson::key("id",
                       writer)    &&
        rapidjson::string(obj.id,
                          writer) &&
        rapidjson::key("properties",
                       writer)    &&
        writer.StartArray()))
  {
    return false;
  }
  for (auto&& property : obj.properties) {
    if (!yggdrasil::to_json(property,
                            writer))
    {
      return false;
    }
  }
  return writer.EndArray() &&
         writer.EndObject();
}

/**
 *  Receives SAX events by being a RapidJSON `Reader`
 *  and transforms them into a \ref user object if
 *  appropriate.
 */
class user_parser
#ifndef MCPP_DOXYGEN_RUNNING
: public rapidjson::state_machine_parser_base<rapidjson::string_parser,
                                              rapidjson::array_parser<property,
                                                                      property_parser>>
#endif
{
private:
  using base = rapidjson::state_machine_parser_base<rapidjson::string_parser,
                                                    rapidjson::array_parser<property,
                                                                            property_parser>>;
public:
  /**
   *  Creates a new user_parser which parses
   *  into a certain \ref user object.
   *
   *  \param [in] u
   *    A reference to the \ref user object into
   *    which to parse. Reference must remain valid until members
   *    of the newly-created object are no longer liable to be
   *    called or the behavior is undefined.
   */
  explicit user_parser(user& u) noexcept;
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
  user* obj_;
  bool  begin_;
  bool  end_;
  bool  id_;
  bool  properties_;
};

}
