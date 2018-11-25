/**
 *  \file
 */

#pragma once

#include <cassert>
#include <type_traits>
#include <utility>
#include <boost/core/noncopyable.hpp>

namespace mcpp {

/**
 *  Uses a policy (which may be stateful) to manage
 *  a handle. The handle is expected to be trivially
 *  movable and non-copyable.
 *
 *  \tparam Policy
 *    A policy type which may be stateful (i.e. an
 *    instance of this type is attached to each instance
 *    of the resulting class) and in terms of which
 *    operations on the resulting class are defined.
 */
template<typename Policy>
class handle : private boost::noncopyable,
               private Policy
{
public:
  /**
   *  A type alias for the `native_handle_type` type
   *  alias from the `Policy` template parameter.
   */
  using native_handle_type = typename Policy::native_handle_type;
  /**
   *  A type alias for the `const_native_handle_type`
   *  type alias from the `Policy` template parameter.
   */
  using const_native_handle_type = typename Policy::const_native_handle_type;
  /**
   *  Creates an instance of `native_handle_type` by
   *  calling `create` on a `Policy` object.
   *
   *  \param [in] policy
   *    The `Policy` object to use to implement the
   *    operations on this class. Optional. Defaults
   *    to a default constructed instance.
   */
  explicit handle(const Policy& policy = Policy())
    : Policy (policy),
      handle_(Policy::create())
  {
    assert(Policy::valid(handle_));
  }
  /**
   *  Creates an instance by assuming ownership of
   *  a handle.
   *
   *  \param [in] handle
   *    The handle to assume ownership of.
   *  \param [in] policy
   *    The `Policy` object to use to implement
   *    operations on the new instance. Optional.
   *    Defaults to a default constructed instance.
   */
  explicit handle(native_handle_type handle,
                  const Policy& policy = Policy()) noexcept
    : Policy (policy),
      handle_(handle)
  {
    assert(Policy::valid(handle_));
  }
  /**
   *  Assumes ownership of the handle managed by
   *  another instance.
   *
   *  The other object is left in an unspecified
   *  but valid state (i.e. it is safe to assign
   *  to and destroy but nothing else).
   *
   *  \param [in] other
   *    The object from which to assume ownership.
   */
  handle(handle&& other) noexcept
    : Policy (other),
      handle_(other.handle_)
  {
    other.handle_ = Policy::invalid();
  }
  /**
   *  Assumes ownership of the handle managed by
   *  another instance by replacing the handle
   *  managed by this instance.
   *
   *  The other object is left in an unspecified
   *  but valid state (i.e. it is safe to assign
   *  to and destroy but nothing else).
   *
   *  \param [in] rhs
   *    The object from which to assume ownership.
   *
   *  \return
   *    A reference to this object.
   */
  handle& operator=(handle&& rhs) noexcept {
    assert(this != &rhs);
    destroy();
    static_cast<Policy&>(*this) = std::move(rhs);
    handle_ = rhs.handle_;
    rhs.handle_ = Policy::invalid();
    return *this;
  }
  /**
   *  Cleans up the managed handle. If calling
   *  `valid` on the contained `Policy` with the
   *  managed handle returns `false` nothing happens,
   *  otherwise `destroy` is called on the contained
   *  `Policy` with the managed handle.
   */
  ~handle() noexcept {
    if (Policy::valid(handle_)) {
      Policy::destroy(handle_);
    }
  }
  /**
   *  @{
   *  Retrieves the managed handle.
   *
   *  \return
   *    The managed handle.
   */
  native_handle_type native_handle() noexcept {
    assert(Policy::valid(handle_));
    return handle_;
  }
  const_native_handle_type native_handle() const noexcept {
    assert(Policy::valid(handle_));
    return handle_;
  }
  /**
   *  @}
   */
private:
  void destroy() noexcept {
    if (Policy::valid(handle_)) {
      Policy::destroy(handle_);
    }
  }
  native_handle_type handle_;
};

/**
 *  A convenience base class for implementing classes
 *  suitable for implementing `Policy` type parameters
 *  to the \ref handle class template.
 *
 *  \tparam Handle
 *    The pointer handle type.
 */
template<typename Handle>
class pointer_handle_policy_base {
public:
  /**
   *  A type alias for the `Handle` template parameter.
   */ 
  using native_handle_type = Handle;
  /**
   *  A type alias for the result of stripping pointer
   *  qualification from `native_handle_type`, adding
   *  const-qualification thereto, and then re-adding
   *  pointer qualification.
   */
  using const_native_handle_type = std::add_pointer_t<std::add_const_t<std::remove_pointer_t<Handle>>>;
  /**
   *  Determines whether or not a handle is valid by
   *  determining whether or not it is `nullptr`.
   *
   *  \param [in] handle
   *    The handle to check.
   *
   *  \return
   *    `true` if `handle` is not `nullptr`, `false`
   *    otherwise.
   */
  static constexpr bool valid(const_native_handle_type handle) noexcept {
    return !!handle;
  }
  /**
   *  Retrieves an invalid handle, i.e. `nullptr`.
   *
   *  \return
   *    `nullptr`.
   */
  static constexpr native_handle_type invalid() noexcept {
    return nullptr;
  }
};

}
