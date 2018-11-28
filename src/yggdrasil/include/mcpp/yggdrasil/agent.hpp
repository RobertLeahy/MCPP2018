/**
 *  \file
 */

#pragma once

#include <cstddef>
#include <string>
#include <mcpp/rapidjson/key.hpp>
#include <mcpp/rapidjson/state_machine_parser_base.hpp>
#include <mcpp/rapidjson/string.hpp>
#include <mcpp/rapidjson/string_parser.hpp>
#include <mcpp/rapidjson/uint_parser.hpp>

namespace mcpp::yggdrasil {

/**
 *  An agent object to be sent to (and received by)
 *  the Yggdrasil API. Describes the requesting
 *  application.
 */
class agent {
public:
  /**
   *  The name of the requesting application. Known
   *  valid values are:
   *
   *  - &quot;Minecraft&quot;
   *  - &quot;Scrolls&quot;
   */
  std::string name;
  /**
   *  Currently always `1`.
   */
  unsigned version = 1;
};

/**
 *  Transforms an \ref agent object into a sequence
 *  of SAX events.
 *
 *  \tparam Writer
 *    A RapidJSON-compatible `Writer` type.
 *
 *  \param [in] obj
 *    The \ref agent object.
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
bool to_json(const agent& obj,
             Writer& writer)
{
  return writer.StartObject()      &&
         rapidjson::key("name",
                        writer)    &&
         rapidjson::string(obj.name,
                           writer) &&
         rapidjson::key("version",
                        writer)    &&
         writer.Uint(obj.version)  &&
         writer.EndObject();
}

/**
 *  A read-only RapidJSON `Reader` which parses
 *  \ref agent objects from a series of SAX
 *  events.
 */
class agent_parser
#ifndef MCPP_DOXYGEN_RUNNING
: public rapidjson::state_machine_parser_base<rapidjson::string_parser,
                                              rapidjson::uint_parser>
#endif
{
private:
  using base = rapidjson::state_machine_parser_base<rapidjson::string_parser,
                                                    rapidjson::uint_parser>;
public:
  /**
   *  Creates an agent_parser object which parses
   *  into a certain \ref agent object.
   *
   *  \param [in] a
   *    A reference to an \ref agent object into
   *    which the parse shall take place. The reference
   *    must remain valid until the last SAX event is
   *    received by the newly-constructed object or
   *    the behavior is undefined.
   */
  explicit agent_parser(agent& a) noexcept;
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
  agent* obj_;
  bool   begin_;
  bool   end_;
  bool   name_;
  bool   version_;
};

}
