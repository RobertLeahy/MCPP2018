#include <mcpp/rapidjson/bool_parser.hpp>

#include <cassert>

namespace mcpp::rapidjson {

bool_parser::bool_parser(bool& b) noexcept
  : b_(&b)
{}

bool bool_parser::Bool(bool b) noexcept {
  assert(b_);
  assert(!done());
  *b_ = b;
  finish();
  return true;
}

}
