/** 
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>
#include <system_error>
#include <boost/asio/buffer.hpp>
#include <boost/core/noncopyable.hpp>
#include <mcpp/checked.hpp>
#include <zlib.h>

namespace mcpp::zlib {

namespace detail {

enum class stream_error {
  success = 0,
  avail_in_overflow,
  avail_out_overflow
};

std::error_code make_error_code(stream_error) noexcept;

template<typename Allocator>
auto get_allocator(void* opaque) noexcept {
  assert(opaque);
  auto&& alloc = *static_cast<const Allocator*>(opaque);
  return typename std::allocator_traits<Allocator>::template rebind_alloc<std::byte>(alloc);
}

template<typename Allocator>
void* alloc(void* opaque,
            ::uInt items,
            ::uInt size) noexcept
{
  auto alloc = detail::get_allocator<Allocator>(opaque);
  using traits_type = std::allocator_traits<decltype(alloc)>;
  using size_type = typename traits_type::size_type;
  auto bytes = mcpp::checked_cast<size_type>(mcpp::checked_add(sizeof(std::size_t),
                                                               mcpp::checked_multiply(items,
                                                                                      size)));
  if (!bytes) {
    return nullptr;
  }
  std::byte* ptr;
  try {
    ptr = traits_type::allocate(alloc,
                                *bytes);
  } catch (...) {
    return nullptr;
  }
  //  TODO: Align?
  std::memcpy(ptr,
              std::addressof(*bytes),
              sizeof(*bytes));
  return ptr + sizeof(*bytes);
}

template<typename Allocator>
void free(void* opaque,
          void* address) noexcept
{
  assert(opaque);
  assert(address);
  auto alloc = detail::get_allocator<Allocator>(opaque);
  using traits_type = std::allocator_traits<decltype(alloc)>;
  using size_type = typename traits_type::size_type;
  size_type size;
  std::byte* bytes = reinterpret_cast<std::byte*>(address);
  bytes -= sizeof(size);
  std::memcpy(&size,
              bytes,
              sizeof(size));
  traits_type::deallocate(alloc,
                          bytes,
                          size);
}

}

template<typename Allocator>
class basic_stream : private boost::noncopyable {
public:
  /**
   *  Type alias for the `Allocator` template
   *  parameter.
   */
  using allocator_type = Allocator;
  /**
   *  Type alias for `z_streamp` (note that this is
   *  `z_stream*` not `z_stream` given that zlib
   *  C API functions accept a pointer).
   */
  using native_handle_type = ::z_streamp;
  /**
   *  Creates a new stream and associates an allocator
   *  with it. It is not initialized by any zlib call.
   *
   *  \param [in] alloc
   *    The `ProtoAllocator`.
   */
  explicit basic_stream(const allocator_type& alloc) noexcept
    : alloc_(alloc)
  {
    stream_.opaque = const_cast<Allocator*>(std::addressof(alloc));
    stream_.zalloc = &detail::alloc<Allocator>;
    stream_.zfree = &detail::free<Allocator>;
    reset();
  }
  /**
   *  Retrieves a pointer to the managed
   *  `z_stream` object.
   *
   *  \return
   *    A \ref native_handle_type.
   */
  native_handle_type native_handle() noexcept {
    return &stream_;
  }
  /**
   *  Retrieves the associated allocator.
   *
   *  \return
   *    A `ProtoAllocator`.
   */
  allocator_type get_allocator() const noexcept {
    return alloc_;
  }
  /**
   *  Resets the `next_in`, `avail_in`, `next_out`,
   *  and `avail_out` members of the wrapped `z_stream`.
   */
  void reset() noexcept {
    stream_.next_in = nullptr;
    stream_.avail_in = 0;
    stream_.next_out = nullptr;
    stream_.avail_out = 0;
  }
  /**
   *  Sets `next_in` and `avail_in` to the appropriate
   *  pointer and size for a certain `boost::asio::const_buffer`.
   *
   *  \param [in] buffer
   *    The buffer from which bytes shall be drawn.
   *
   *  \return
   *    A `std::error_code` indicating the result of the operation.
   */
  std::error_code in(boost::asio::const_buffer buffer) noexcept {
    auto avail_in = mcpp::checked_cast<::uInt>(buffer.size());
    if (!avail_in) {
      return make_error_code(detail::stream_error::avail_in_overflow);
    }
    stream_.avail_in = *avail_in;
    stream_.next_in = static_cast<::Bytef*>(const_cast<void*>(buffer.data()));
    return std::error_code();
  }
  /**
   *  Retrieves a `boost::asio::const_buffer` representing the
   *  remaining input sequence.
   *
   *  \return
   *    A `boost::asio::const_buffer` object which refers to the
   *    remaining input sequence (i.e. the bytes which are available
   *    to be processed by zlib).
   */
  boost::asio::const_buffer in() const noexcept {
    return boost::asio::const_buffer(stream_.next_in,
                                     std::size_t(stream_.avail_in));
  }
  /**
   *  Sets `next_out` and `avail_out` to the appropriate
   *  pointer and size for a certain `boost::asio::mutable_buffer`.
   *
   *  \param [in] buffer
   *    The buffer to which bytes shall be written.
   *
   *  \return
   *    A `std::error_code` indicating the result of the operation.
   */
  std::error_code out(boost::asio::mutable_buffer buffer) noexcept {
    auto avail_out = mcpp::checked_cast<::uInt>(buffer.size());
    if (!avail_out) {
      return make_error_code(detail::stream_error::avail_out_overflow);
    }
    stream_.avail_out = *avail_out;
    stream_.next_out = static_cast<::Bytef*>(const_cast<void*>(buffer.data()));
    return std::error_code();
  }
  /**
   *  Retrieves a `boost::asio::mutable_buffer` representing the
   *  remaining output buffer.
   *
   *  \return
   *    A `boost::asio::const_buffer` object which refers to the
   *    remaining input buffer (i.e. the bytes which are available
   *    to be written to by zlib).
   */
  boost::asio::mutable_buffer out() const noexcept {
    return boost::asio::mutable_buffer(stream_.next_out,
                                       std::size_t(stream_.avail_out));
  }
private:
  ::z_stream     stream_;
  allocator_type alloc_;
};

/**
 *  A type alias for \ref basic_stream with
 *  `std::allocator<void>` as the `Allocator` template
 *  parameter.
 */
using stream = basic_stream<std::allocator<void>>;

}
