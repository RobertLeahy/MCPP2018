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
 *  compression.
 *
 *  \tparam Allocator
 *    A model of the `ProtoAllocator` concept.
 */
template<typename Allocator>
class basic_deflate_stream : public basic_stream<Allocator> {
private:
  using base = basic_stream<Allocator>;
public:
  /**
   *  Associates an `Allocator` with the internal
   *  `z_stream` and calls `deflateInit` thereupon.
   *
   *  \param [in] level
   *    The second argument to `deflateInit`. Defaults
   *    to `Z_DEFAULT_COMPRESSION`.
   *  \param [in] alloc
   *    The `Allocator` to associate with the stream.
   *    Defaults to a default constructed
   *    \ref allocator_type.
   */
  explicit basic_deflate_stream(int level = Z_DEFAULT_COMPRESSION,
                                const Allocator& alloc = Allocator())
    : base(alloc)
  {
    int result = ::deflateInit(base::native_handle(),
                               level);
    if (result != Z_OK) {
      throw std::system_error(zlib::make_error_code(result));
    }
  }
  /**
   *  Associates an `Allocator` with the internal
   *  `z_stream` and calls `deflateInit2` thereupon.
   *
   *  \param [in] level
   *    The `level` parameter to `deflateInit2`.
   *  \param [in] method
   *    The `method` parameter to `deflateInit2`.
   *  \param [in] window_bits
   *    The `windowBits` parameter to `deflateInit2`.
   *  \param [in] mem_level
   *    The `memLevel` parameter to `deflateInit2`.
   *  \param [in] strategy
   *    The `strategy` parameter to `deflateInit2`.
   *  \param [in] alloc
   *    The `Allocator` to associate with the stream.
   *    Defaults to a default constructed
   *    \ref allocator_type.
   */
  explicit basic_deflate_stream(int level,
                                int method,
                                int window_bits,
                                int mem_level,
                                int strategy,
                                const Allocator& alloc = Allocator())
    : base(alloc)
  {
    int result = ::deflateInit2(base::native_handle(),
                                level,
                                method,
                                window_bits,
                                mem_level,
                                strategy);
    if (result != Z_OK) {
      throw std::system_error(zlib::make_error_code(result));
    }
  }
  /**
   *  Calls `deflateEnd`.
   */
  ~basic_deflate_stream() noexcept {
    int result = ::deflateEnd(base::native_handle());
    assert(result == Z_OK);
    (void)result;
  }
  /**
   *  Calls `deflateReset` after resetting the stream's
   *  buffer states to empty.
   */
  void reset() noexcept {
    base::reset();
    int result = ::deflateReset(base::native_handle());
    assert(result == Z_OK);
    (void)result;
  }
};

/**
 *  A type alias for \ref basic_deflate_stream with
 *  `std::allocator<void>` as the `Allocator` template
 *  parameter.
 */
using deflate_stream = basic_deflate_stream<std::allocator<void>>;

}
