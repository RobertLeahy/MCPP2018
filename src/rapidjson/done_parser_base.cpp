#include <mcpp/rapidjson/done_parser_base.hpp>

#include <cassert>

namespace mcpp::rapidjson {

done_parser_base::done_parser_base() noexcept
  : done_(false)
{}

bool done_parser_base::done() const noexcept {
  return done_;
}

void done_parser_base::clear() noexcept {
  parser_base::clear();
  done_ = false;
}

void done_parser_base::finish() noexcept {
  assert(!done_);
  done_ = true;
}

}
