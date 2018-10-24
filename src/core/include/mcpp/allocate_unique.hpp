/**
 *  \file
 */

#pragma once

#include <memory>
#include <optional>
#include <utility>
#include <boost/core/noncopyable.hpp>

namespace mcpp {

namespace detail {

template<typename Allocator>
class allocate_unique_deleter {
public:
  using allocator_type = Allocator;
private:
  using traits_type = std::allocator_traits<allocator_type>;
public:
  using type = typename traits_type::pointer;
  explicit allocate_unique_deleter(const allocator_type& alloc) noexcept
    : alloc_(alloc)
  {}
  void operator()(type ptr) const noexcept {
    allocator_type alloc(alloc_);
    traits_type::destroy(alloc,
                         std::addressof(*ptr));
    traits_type::deallocate(alloc,
                            ptr,
                            1);
  }
private:
  allocator_type alloc_;
};

}

/**
 *  The type returned by \ref allocate_unique.
 *
 *  An instantiation of `std::unique_ptr` parameterized with
 *  `T` and an implementation-defined `Deleter` type which
 *  wraps `Allocator`.
 *
 *  \tparam T
 *    The pointee type.
 *  \tparam Allocator
 *    A type which models `Allocator`.
 */
template<typename T,
         typename Allocator>
using allocate_unique_t = std::unique_ptr<T,
                                          detail::allocate_unique_deleter<typename std::allocator_traits<Allocator>::template rebind_alloc<T>>>;

/**
 *  Creates a `std::unique_ptr` which manages an object of
 *  type `T` residing in memory allocated from an `Allocator`
 *  of type `Allocator`.
 *
 *  \tparam T
 *    The pointee type.
 *  \tparam Allocator
 *    A type which models `Allocator`. If this `Allocator` does
 *    not directly allocate memory for objects of type `T` it
 *    will be rebound.
 *  \tparam Args
 *    The types of arguments to use to construct a `T`.
 *
 *  \param [in] alloc
 *    The `Allocator` to use to allocate memory for an object
 *    of type `T`.
 *  \param [in] args
 *    Arguments to forward through to a constructor of `T`.
 *
 *  \return
 *    A `std::unique_ptr` which manages an object of type `T`
 *    constructed from `args` residing in memory allocated by
 *    `alloc` with a `Deleter` which destroys the object of type
 *    `T` and releases the memory in which it resides back to
 *    a copy of `alloc`.
 */
template<typename T,
         typename Allocator,
         typename... Args>
allocate_unique_t<T,
                  Allocator> allocate_unique(const Allocator& alloc,
                                             Args&&... args)
{
  using return_type = allocate_unique_t<T,
                                        Allocator>;
  using deleter_type = typename return_type::deleter_type;
  using allocator_type = typename deleter_type::allocator_type;
  using pointer = typename return_type::pointer;
  using traits_type = std::allocator_traits<allocator_type>;
  allocator_type a(alloc);
  pointer p = traits_type::allocate(a,
                                    1);
  class guard : private boost::noncopyable {
  public:
    guard(const allocator_type& a,
          pointer p) noexcept
      : a_(a),
        p_(p)
    {}
    ~guard() noexcept {
      if (p_) {
        traits_type::deallocate(a_,
                                *p_,
                                1);
      }
    }
    void release() noexcept {
      p_ = std::nullopt;
    }
  private:
    allocator_type         a_;
    std::optional<pointer> p_;
  };
  guard g(a,
          p);
  traits_type::construct(a,
                         p,
                         std::forward<Args>(args)...);
  g.release();
  return return_type(p,
                     deleter_type(a));
}

}
