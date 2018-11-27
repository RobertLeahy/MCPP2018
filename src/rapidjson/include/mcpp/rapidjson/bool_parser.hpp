/**
 *  \file
 */

#pragma once

#include "done_parser_base.hpp"

namespace mcpp::rapidjson {

/**
 *  Parses `bool` objects from JSON SAX events.
 */
class bool_parser
#ifndef MCPP_DOXYGEN_RUNNING
: public done_parser_base
#endif
{
public:
  /**
   *  Creates a parser which parses into a
   *  certain `bool` object.
   *
   *  \param [in] b
   *    A reference to an `bool` object into
   *    which to parse. This reference must
   *    remain valid until the newly-constructed
   *    object finishes receiving SAX events.
   */
  explicit bool_parser(bool& b) noexcept;
#ifndef MCPP_DOXYGEN_RUNNING
  bool Bool(bool) noexcept;
#endif
private:
  bool* b_;
};

}
