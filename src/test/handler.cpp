#include <mcpp/test/handler.hpp>

#include <cassert>
#include <stdexcept>
#include <mcpp/system_error.hpp>

namespace mcpp::test {

handler_state::handler_state() noexcept
  : invoked(false)
{}

void handler_state::clear () noexcept {
  invoked = false;
}

handler::handler(handler_state& state) noexcept
  : state_(&state)
{}

void handler::operator()() {
  assert(state_);
  if (state_->invoked) {
    throw std::logic_error("Already invoked");
  }
  state_->invoked = true;
}

void completion_handler_state::clear() noexcept {
  handler_state::clear();
  ec.clear();
}

completion_handler::completion_handler(completion_handler_state& state) noexcept
  : handler(state),
    state_ (&state)
{}

void completion_handler::operator()(boost::system::error_code ec) {
  (*this)(to_error_code(ec));
}

void completion_handler::operator()(std::error_code ec) {
  assert(state_);
  static_cast<handler&>(*this)();
  state_->ec = ec;
}

read_handler_state::read_handler_state() noexcept
  : bytes_transferred(0)
{}

void read_handler_state::clear() noexcept {
  completion_handler_state::clear();
  bytes_transferred = 0;
}

read_handler::read_handler(read_handler_state& state) noexcept
  : completion_handler(state),
    state_            (&state)
{}

}
