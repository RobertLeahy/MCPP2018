/**
 *  \file
 */

#pragma once

#include "done_parser_base.hpp"

namespace mcpp::rapidjson {

/**
 *  Parses `unsigned` objects from JSON SAX events.
 */
class uint_parser
#ifndef MCPP_DOXYGEN_RUNNING
: public done_parser_base
#endif
{
public:
  /**
   *  Creates a parser which parses into a
   *  certain `unsigned` object.
   *
   *  \param [in] u
   *    A reference to an `unsigned` object into
   *    which to parse. This reference must
   *    remain valid until the newly-constructed
   *    object finishes receiving SAX events.
   */
  explicit uint_parser(unsigned& u) noexcept;
#ifndef MCPP_DOXYGEN_RUNNING
  bool Uint(unsigned) noexcept;
#endif
private:
  unsigned* u_;
};

}
