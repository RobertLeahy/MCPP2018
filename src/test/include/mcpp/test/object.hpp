#pragma once

#include <cstddef>
#include <exception>
#include <boost/core/noncopyable.hpp>

namespace mcpp::test {

class object {
public:
  class state : private boost::noncopyable {
  public:
    state() noexcept;
    std::size_t        construct;
    std::size_t        copy_construct;
    std::size_t        move_construct;
    std::size_t        copy_assign;
    std::size_t        move_assign;
    std::size_t        destruct;
    std::exception_ptr construct_ex;
    std::exception_ptr copy_construct_ex;
    std::exception_ptr move_construct_ex;
    std::exception_ptr copy_assign_ex;
    std::exception_ptr move_assign_ex;
  };
  explicit object(state& s);
  object(const object& rhs);
  object(object&& rhs);
  object& operator=(const object& rhs);
  object& operator=(object&& rhs);
  ~object() noexcept;
private:
  state* state_;
};

}
