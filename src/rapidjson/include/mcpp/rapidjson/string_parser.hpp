/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include "done_parser_base.hpp"

namespace mcpp::rapidjson {

/**
 *  Parses string objects from JSON SAX events.
 *
 *  \tparam CharT
 *    The character type.
 *  \tparam Traits
 *    The character traits type. Defaults to
 *    `std::char_traits<CharT>`.
 *  \tparam Allocator
 *    The `Allocator`. Defaults to `std::allocator<CharT>`.
 */
template<typename CharT,
         typename Traits = std::char_traits<CharT>,
         typename Allocator = std::allocator<CharT>>
class basic_string_parser
#ifndef MCPP_DOXYGEN_RUNNING
: public done_parser_base
#endif
{
public:
  /**
   *  An instantiation of `std::basic_string` appropriate
   *  for the template arguments used to instantiate this
   *  class template.
   */
  using string_type = std::basic_string<CharT,
                                        Traits,
                                        Allocator>;
  /**
   *  Creates a parser which parses into a certain
   *  string object.
   *
   *  \param [in] str
   *    A reference to the \ref string_type "string"
   *    into which to parse. This reference must remain
   *    valid until the return from the last SAX event this
   *    object receives.
   */
  explicit basic_string_parser(string_type& str) noexcept
    : str_(&str)
  {}
#ifndef MCPP_DOXYGEN_RUNNING
  bool String(const CharT* str,
              std::size_t size,
              bool)
  {
    assert(str_);
    assert(!size || str);
    assert(!done());
    str_->assign(str,
                 str + size);
    finish();
    return true;
  }
#endif
private:
  string_type* str_;
};

/**
 *  A type alias for \ref basic_string_parser with
 *  `char` as the first template argument and the
 *  others as defaults. The relationship between this
 *  type alias and \ref basic_string_parser is
 *  analogous to the relationship between `std::basic_string`
 *  and `std::string`.
 */
using string_parser = basic_string_parser<char>;

}
