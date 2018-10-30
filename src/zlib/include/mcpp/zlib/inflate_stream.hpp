/**
 *  \file
 */

#pragma once

#include <cassert>
#include <memory>
#include <system_error>
#include <boost/core/noncopyable.hpp>
#include "stream.hpp"
#include "system_error.hpp"
#include <zlib.h>

namespace mcpp::zlib {

/**
 *  Wraps a `z_stream` in an RAII class and associates
 *  a C++ allocator with it for the purpose of
 *  decompression.
 *
 *  \tparam Allocator
 *    A model of the `ProtoAllocator` concept.
 */
template<typename Allocator>
class basic_inflate_stream : public basic_stream<Allocator> {
private:
  using base = basic_stream<Allocator>;
public:
  /**
   *  Associates an `Allocator` with the internal
   *  `z_stream` and calls `inflateInit` thereupon.
   *
   *  \param [in] alloc
   *    The `Allocator` to associate with the stream.
   *    Defaults to a default constructed
   *    \ref allocator_type.
   */
  explicit basic_inflate_stream(const Allocator& alloc = Allocator())
    : base(alloc)
  {
    int result = ::inflateInit(base::native_handle());
    if (result != Z_OK) {
      throw std::system_error(zlib::make_error_code(result));
    }
  }
  /**
   *  Associates an `Allocator` with the internal
   *  `z_stream` and calls `inflateInit2` thereupon.
   *
   *  \param [in] window_bits
   *    The second argument to `inflateInit2`.
   *  \param [in] alloc
   *    The `Allocator` to associate with the stream.
   *    Defaults to a default constructed
   *    \ref allocator_type.
   */
  explicit basic_inflate_stream(int window_bits,
                                const Allocator& alloc = Allocator())
    : base(alloc)
  {
    int result = ::inflateInit2(base::native_handle(),
                                window_bits);
    if (result != Z_OK) {
      throw std::system_error(zlib::make_error_code(result));
    }
  }
  /**
   *  Calls `inflateEnd`.
   */
  ~basic_inflate_stream() noexcept {
    int result = ::inflateEnd(base::native_handle());
    assert(result == Z_OK);
    (void)result;
  }
  /**
   *  Calls `inflateReset` after resetting the stream's
   *  buffer states to empty.
   */
  void reset() noexcept {
    base::reset();
    int result = ::inflateReset(base::native_handle());
    assert(result == Z_OK);
    (void)result;
  }
};

/**
 *  A type alias for \ref basic_inflate_stream with
 *  `std::allocator<void>` as the `Allocator` template
 *  parameter.
 */
using inflate_stream = basic_inflate_stream<std::allocator<void>>;

}
