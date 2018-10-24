#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <boost/core/noncopyable.hpp>

namespace mcpp::test {

class allocator_state : private boost::noncopyable {
public:
  allocator_state() noexcept;
  std::size_t allocate;
  std::size_t deallocate;
};

template<typename T>
class allocator : public std::allocator<T> {
template<typename>
friend class allocator;
private:
  using base = std::allocator<T>;
public:
  using state = allocator_state;
  explicit allocator(state& state) noexcept
    : state_(&state)
  {}
  template<typename U>
  explicit allocator(const allocator<U>& other) noexcept
    : base  (other),
      state_(other.state_)
  {
    assert(state_);
  }
private:
  using traits_type = std::allocator_traits<base>;
public:
  using pointer = typename traits_type::pointer;
  using size_type = typename traits_type::size_type;
  pointer allocate(size_type n) {
    assert(state_);
    ++state_->allocate;
    return base::allocate(n);
  }
  void deallocate(pointer p,
                  size_type n) noexcept
  {
    assert(state_);
    ++state_->deallocate;
    return base::deallocate(p,
                            n);
  }
  template<typename U>
  class rebind {
  public:
    using other = allocator<U>;
  };
private:
  state* state_;
};

template<>
class allocator<void> : public std::allocator<void> {
template<typename>
friend class allocator;
public:
  using state = allocator_state;
  explicit allocator(state& state) noexcept;
  template<typename U>
  explicit allocator(const allocator<U>& other) noexcept
    : std::allocator<void>(other),
      state_              (other.state_)
  {
    assert(state_);
  }
  template<typename U>
  class rebind {
  public:
    using other = allocator<U>;
  };
private:
  state* state_;
};

}
