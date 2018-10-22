/**
 *  \file
 */

#pragma once

#include <memory>
#include <utility>
#include <mcpp/any_storage.hpp>

namespace mcpp {

/**
 *  Provides storage and a pointer-like interface
 *  to classes which are derived from a common
 *  base class.
 *
 *  \tparam Base
 *    The common base class of all objects which
 *    shall be stored.
 */
template<typename Base>
class polymorphic_ptr : public any_storage {
template<typename>
friend class polymorphic_ptr;
public:
  /**
   *  A type alias for the pointer type used by
   *  objects of this type.
   */
  using pointer = Base*;
  /**
   *  A type alias for the template parameter to
   *  the class template.
   */
  using element_type = Base;
  using any_storage::any_storage;
  /**
   *  Allows polymorphic_ptr objects to be constructed
   *  by moving from polymorphic_ptr objects instantiated
   *  with a different template argument which is covariant
   *  with `Base`.
   *
   *  \tparam Other
   *    The template parameter type of the other
   *    polymorphic_ptr.
   *
   *  \param [in] other
   *    The polymorphic_ptr to move from.
   */
  template<typename Other>
  polymorphic_ptr(polymorphic_ptr<Other>&& other) noexcept
    : any_storage(std::move(other)),
      ptr_       (other.ptr_)
  {}
  /**
   *  Allows polymorphic_ptr objects to be assigned
   *  by moving from polymorphic_ptr objects instantiated
   *  with a different template argument which is covariant
   *  with `Base`.
   *
   *  \tparam Other
   *    The template parameter type of the other
   *    polymorphic_ptr.
   *
   *  \param [in] rhs
   *    The polymorphic_ptr to move from.
   *
   *  \return
   *    A reference to this object.
   */
  template<typename Other>
  polymorphic_ptr& operator=(polymorphic_ptr<Other>&& rhs) noexcept {
    static_cast<any_storage&>(*this) = std::move(rhs);
    ptr_ = rhs.ptr_;
    return *this;
  }
  /**
   *  Constructs an instance of a derived type in the
   *  managed storage.
   *
   *  \tparam Derived
   *    The type to construct. Must be covariant with
   *    `Base`.
   *  \tparam Args
   *    The types to be used to construct an object of
   *    type `Derived`.
   *
   *  \param [in] args
   *    The arguments to use to consturct an object of
   *    type `Derived`.
   *
   *  \return
   *    A reference to the newly-managed object upcast
   *    to `Base&`.
   */
  template<typename Derived,
           typename... Args>
  element_type& emplace(Args&&... args) {
    ptr_ = std::addressof(any_storage::emplace<Derived>(std::forward<Args>(args)...));
    return *ptr_;
  }
  /**
   *  Obtains a reference to the managed object.
   *
   *  If there is no managed object the behavior is
   *  undefined.
   *
   *  \return
   *    A reference to the managed object upcast to
   *    `Base&`.
   */
  element_type& operator*() const noexcept {
    assert(has_value());
    return *ptr_;
  }
  /**
   *  @{
   *
   *  Obtains a pointer to the managed object.
   *
   *  If there is no managed object the behavior is
   *  undefined.
   *
   *  \return
   *    A pointer to the managed object upcast to
   *    `Base*`.
   */
  pointer operator->() const noexcept {
    assert(has_value());
    return ptr_;
  }
  pointer get() const noexcept {
    assert(has_value());
    return ptr_;
  }
  /**
   *  @}
   */
private:
  Base* ptr_;
};

}
