#include <mcpp/test/object.hpp>

#include <cassert>
#include <exception>
#include <utility>

namespace mcpp::test {

object::state::state() noexcept
  : construct     (0),
    copy_construct(0),
    move_construct(0),
    copy_assign   (0),
    move_assign   (0),
    destruct      (0)
{}

namespace {

void throw_if(std::exception_ptr ex) {
  if (ex) {
    std::rethrow_exception(std::move(ex));
  }
}

}

object::object(state& s)
  : state_(&s)
{
  ++state_->construct;
  throw_if(state_->construct_ex);
}

object::object(const object& rhs)
  : state_(rhs.state_)
{
  ++state_->copy_construct;
  throw_if(state_->copy_construct_ex);
}

object::object(object&& rhs)
  : state_(rhs.state_)
{
  ++state_->move_construct;
  throw_if(state_->move_construct_ex);
}

object& object::operator=(const object& rhs) {
  assert(state_);
  state_ = rhs.state_;
  ++state_->copy_assign;
  throw_if(state_->copy_assign_ex);
  return *this;
}

object& object::operator=(object&& rhs) {
  assert(state_);
  state_ = rhs.state_;
  ++state_->move_assign;
  throw_if(state_->move_assign_ex);
  return *this;
}

object::~object() noexcept {
  assert(state_);
  ++state_->destruct;
}

}
