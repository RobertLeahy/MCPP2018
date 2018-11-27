#include <mcpp/rapidjson/uint_parser.hpp>

#include <cassert>

namespace mcpp::rapidjson {

uint_parser::uint_parser(unsigned& u) noexcept
  : u_(&u)
{}

bool uint_parser::Uint(unsigned u) noexcept {
  assert(!done());
  assert(u_);
  *u_ = u;
  finish();
  return true;
}

}
