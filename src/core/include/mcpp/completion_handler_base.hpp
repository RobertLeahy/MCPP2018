/**
 *  \file
 */

#pragma once

#include <type_traits>
#include <utility>
#include <boost/asio/associated_allocator.hpp>
#include <boost/asio/associated_executor.hpp>

namespace mcpp {

/**
 *  Stores a completion handler and passes through its
 *  associated `Executor` and `ProtoAllocator`. If no `Executor`
 *  is associated with the completion handler then a fallback
 *  `Executor` specified by the template parameter `Executor`
 *  is associated. If no `ProtoAllocator` is associated with the
 *  completion handler then `std::allocator<void>` is provided.
 *
 *  \tparam Executor
 *    The type of the fallback `Executor`.
 *  \tparam CompletionHandler
 *    The type of the completion handler.
 */
template<typename Executor,
         typename CompletionHandler>
class completion_handler_base {
public:
  /**
   *  @{
   *
   *  Creates a completion_handler_base with a certain
   *  fallback `Executor` and a certain completion handler.
   *
   *  \param [in] ex
   *    The `Executor`.
   *  \param [in] h
   *    The completion handler.
   */
  completion_handler_base(const Executor& ex,
                          CompletionHandler&& h) noexcept(std::is_nothrow_move_constructible_v<CompletionHandler>)
    : ex_(ex),
      h_ (std::move(h))
  {}
  completion_handler_base(const Executor& ex,
                          const CompletionHandler& h) noexcept(std::is_nothrow_copy_constructible_v<CompletionHandler>)
    : ex_(ex),
      h_ (h)
  {}
  /**
   *  @}
   *
   *  Invokes the stored completion handler.
   *
   *  \tparam Args
   *    The types of the arguments with which to invoke
   *    the stored completion handler.
   *
   *  \param [in] args
   *    The arguments with which to invoke the stored
   *    completion handler.
   */
  template<typename... Args>
  void invoke(Args&&... args) noexcept(noexcept(std::declval<CompletionHandler&>()(std::declval<Args>()...))) {
    h_(std::forward<Args>(args)...);
  }
  /**
   *  @{
   *
   *  Retrieves the stored completion handler.
   *
   *  \return
   *    The stored completion handler.
   */
  CompletionHandler& get()& noexcept {
    return h_;
  }
  const CompletionHandler& get() const& noexcept {
    return h_;
  }
  CompletionHandler&& get()&& noexcept {
    return std::move(h_);
  }
  /**
   *  @}
   */
#ifndef MCPP_DOXYGEN_RUNNING
  using executor_type = boost::asio::associated_executor_t<CompletionHandler,
                                                           Executor>;
  using allocator_type = boost::asio::associated_allocator_t<CompletionHandler>;
  auto get_executor() const noexcept {
    return boost::asio::get_associated_executor(h_,
                                                ex_);
  }
  auto get_allocator() const noexcept {
    return boost::asio::get_associated_allocator(h_);
  }
#endif
private:
  Executor          ex_;
  CompletionHandler h_;
};

}
