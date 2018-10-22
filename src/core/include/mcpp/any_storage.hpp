/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <new>
#include <utility>
#include <boost/core/noncopyable.hpp>

namespace mcpp {

/**
 *  Manages storage and allows any object to be
 *  constructed therein.
 *
 *  The managed storage is resized only when
 *  needed.
 */
class any_storage : private boost::noncopyable {
public:
  /**
   *  A type which shall be used to represent
   *  all sizes.
   */
  using size_type = std::size_t;
  /**
   *  Creates an any_storage object which manages
   *  no storage and therefore manages no object.
   */
  any_storage() noexcept;
  /**
   *  Creates an any_storage object which manages
   *  the memory and object of another any_storage
   *  object (the other any_storage thereafter not
   *  managing that memory or object).
   *
   *  \param [in] other
   *    The any_storage object to move from.
   */
  any_storage(any_storage&& other) noexcept;
  /**
   *  Replaces the memory and object managed by
   *  this object with the memory and object of
   *  another any_storage object (which will
   *  thereafter not own that memory or object).
   *
   *  \param [in] rhs
   *    The any_storage object to move from.
   *
   *  \return
   *    A reference to this object.
   */
  any_storage& operator=(any_storage&& rhs) noexcept;
  /**
   *  Frees the managed storage (if any) after
   *  destroying the managed object (if any).
   */
  ~any_storage() noexcept;
  /**
   *  Determines the capacity of the buffer managed
   *  by this object.
   *
   *  \return
   *    The capacity in bytes.
   */
  size_type capacity() const noexcept;
  /**
   *  Destroys the contained value (if any).
   *
   *  The size of the managed storage will not
   *  be affected.
   */
  void reset() noexcept;
  /**
   *  Resizes the managed storage such that it is
   *  at least `new_cap` bytes.
   *
   *  If there is a managed object it shall be
   *  destroyed (i.e. it shall be as if \ref reset
   *  were called).
   *
   *  After this function completes successfully
   *  it shall be the case that for a type `T`, where
   *  `sizeof(T) <= sizeof(new_cap)`, it shall not be
   *  the case that constructing an object of type
   *  `T` in the managed storage causes a reallocation.
   *
   *  \param [in] new_cap
   *    The desired capacity, in bytes.
   */
  void reserve(size_type new_cap);
  /**
   *  @{
   *
   *  Checks to see if this object manages an object.
   *
   *  \return
   *    `true` if this object has an object constructed
   *    in managed storage, `false` otherwise.
   */
  explicit operator bool() const noexcept;
  bool has_value() const noexcept;
  /**
   *  @}
   *
   *  Creates an object in the managed storage resizing
   *  it if necessary.
   *
   *  If there's already an object in the managed storage
   *  it will be destroyed.
   *
   *  \tparam T
   *    The type of object to construct.
   *  \tparam Args
   *    The types of arguments to use to construct an
   *    object of type `T`.
   *
   *  \param [in] args
   *    The arguments to forward to a constructor of
   *    `T`.
   *
   *  \return
   *    A reference to the constructed object.
   */
  template<typename T,
           typename... Args>
  T& emplace(Args&&... args) {
    reserve(sizeof(T));
    assert(!dtor_);
    assert(ptr_);
    assert(size_ >= sizeof(T));
    T* ptr = new (ptr_) T(std::forward<Args>(args)...);
    dtor_ = &dtor<T>;
    return *ptr;
  }
private:
  template<typename T>
  static void dtor(void* ptr) noexcept {
    assert(ptr);
    static_cast<T*>(ptr)->~T();
  }
  void free() noexcept;
  using destructor_type = void (*)(void*) noexcept;
  void*           ptr_;
  size_type       size_;
  destructor_type dtor_;
};

}
