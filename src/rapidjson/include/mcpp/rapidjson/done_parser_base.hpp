/**
 *  \file
 */

#pragma once

#include "parser_base.hpp"

namespace mcpp::rapidjson {

/**
 *  A convenience base class for parsers which
 *  track whether they are finished or not in
 *  terms of a `bool`.
 */
class done_parser_base : public parser_base {
public:
  /**
   *  Initializes the stored `bool` to `false`.
   */
  done_parser_base() noexcept;
  /**
   *  Determines whether the parse is complete.
   *
   *  Simply returns the stored `bool`.
   *
   *  \return
   *    The stored `bool`.
   */
  bool done() const noexcept;
  /**
   *  Calls `clear` on the base class and then
   *  sets the stored `bool` to `false`.
   */
  void clear() noexcept;
protected:
  /**
   *  Sets the stored `bool` to `true`.
   *
   *  If the stored `bool` is already `true`
   *  the behavior is undefined.
   */
  void finish() noexcept;
private:
  bool done_;
};

}
