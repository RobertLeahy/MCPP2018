/**
 *  \file
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <iomanip>
#include <ios>
#include <iterator>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/io/ios_state.hpp>
#include "cast_output_iterator.hpp"

namespace mcpp {

/**
 *  Contains settings for a \ref basic_hex_dump object.
 *
 *  This is defined at namespace scope (rather than within
 *  the class itself) because it does not depend on any
 *  template parameters to the class.
 */
class hex_dump_settings {
public:
  /**
   *  The number of bytes which will be written per line.
   *
   *  Defaults to `16`.
   */
  std::size_t per_line = 16;
  /**
   *  Whether hexadecimal representations of bytes will
   *  be written using uppercase digits (`true`) or
   *  lowercase digits (`false`).
   *
   *  Defaults to `false`.
   */
  bool uppercase = false;
};

/**
 *  Dumps buffers as human readable hex.
 *
 *  \tparam CharT
 *    The character type of the `std::basic_ostream`
 *    instantiation to write to.
 *  \tparam Traits
 *    The character traits type of the `std::basic_ostream`
 *    instantiation to write to. Defaults to
 *    `std::char_traits<CharT>`.
 *  \tparam Allocator
 *    A model of `Allocator` which shall be used to allocate
 *    space to be used to persist partial lines between calls.
 *    Defaults to `std::allocator<void>`.
 */
template<typename CharT,
         typename Traits = std::char_traits<CharT>,
         typename Allocator = std::allocator<void>>
class basic_hex_dump {
public:
  /**
   *  A type alias for the `CharT` template parameter.
   */
  using char_type = CharT;
  /**
   *  A type alias for the `Traits` template parameter.
   */
  using traits_type = Traits;
  /**
   *  The instantiation of `std::basic_ostream` that objects
   *  of this type may write to.
   */
  using ostream_type = std::basic_ostream<char_type,
                                          traits_type>;
  /**
   *  A type alias for the `Allocator` template parameter.
   */
  using allocator_type = Allocator;
  /**
   *  A type alias for the \ref hex_dump_settings class.
   */
  using settings_type = hex_dump_settings;
  /**
   *  Creates a new basic_hex_dump.
   *
   *  \param [in] settings
   *    The \ref settings_type "setting object" to be used
   *    to configure the output of the newly-constructed
   *    object.
   *  \param [in] os
   *    A reference to the \ref ostream_type "output stream"
   *    to write to. This reference must remain valid until
   *    after the last output operation on the newly-constructe
   */
  basic_hex_dump(settings_type settings,
                 ostream_type& os,
                 const allocator_type& alloc = allocator_type())
    : first_   (true),
      settings_(std::move(settings)),
      os_      (os),
      buffer_  (alloc)
  {
    if (!settings_.per_line) {
      throw std::logic_error("per_line == 0");
    }
    buffer_.reserve(settings_.per_line);
  }
  /**
   *  Dumps bytes from a range bounded by two iterators.
   *
   *  Note that not all bytes may be written: Due to the
   *  output format bytes will only be written when a full
   *  line can be written, remaining bytes will be collected
   *  in a buffer to be written on the next operation or when
   *  \ref done is called.
   *
   *  \tparam InputIterator
   *    A model of `InputIterator` from which `std::byte`,
   *    `char`, or `unsigned char` values may be read.
   *
   *  \param [in] begin
   *    An iterator to the first byte.
   *  \param [in] end
   *    An iterator to one past the last byte.
   */
  template<typename InputIterator>
  void operator()(InputIterator begin,
                  InputIterator end)
  {
    assert(settings_.per_line);
    for (; begin != end; ++begin) {
      assert(buffer_.size() < settings_.per_line);
      unsigned char u(*begin);
      buffer_.push_back(u);
      if (buffer_.size() == settings_.per_line) {
        flush();
      }
    }
  }
  /**
   *  Dumps bytes from a sequence of buffers.
   *
   *  Note that not all bytes may be written: Due to the
   *  output format bytes will only be written when a full
   *  line can be written, remaining bytes will be collected
   *  in a buffer to be written on the next operation or when
   *  \ref done is called.
   *
   *  \tparam ConstBufferSequence
   *    A model of `ConstBufferSequence`.
   *
   *  \param [in] cb
   *    A sequence of buffers containing the bytes to write.
   */
  template<typename ConstBufferSequence>
  void operator()(ConstBufferSequence cb) {
    using buffers_iterator_type = boost::asio::buffers_iterator<ConstBufferSequence,
                                                                unsigned char>;
    (*this)(buffers_iterator_type::begin(cb),
            buffers_iterator_type::end(cb));
  }
  /**
   *  Finishes output by flushing bytes from the buffer.
   *
   *  The object is ready to be reused after this function
   *  completes successfully.
   */
  void done() {
    if (!buffer_.empty()) {
      flush();
    }
    first_ = true;
  }
  /**
   *  Retrieves the \ref settings_type "settings object"
   *  used to configure this object.
   *
   *  \return
   *    The \ref settings_type "settings object".
   */
  const settings_type& settings() const noexcept {
    return settings_;
  }
  /**
   *  Retrieves the \ref ostream_type "output stream"
   *  used by this object.
   *
   *  \return
   *    The \ref ostream_type "output stream".
   */
  ostream_type& ostream() const noexcept {
    return os_;
  }
private:
  void flush() {
    assert(!buffer_.empty());
    assert(buffer_.size() <= settings_.per_line);
    boost::io::basic_ios_all_saver<char_type,
                                   traits_type> g(os_);
    if (!first_) {
      os_ << '\n';
    }
    first_ = false;
    std::size_t written = 0;
    for (auto&& u : buffer_) {
      unsigned i(u);
      if (written) {
        os_ << ' ';
      }
      os_ << std::setw(2) << std::setfill('0') << std::hex;
      if (settings_.uppercase) {
        os_ << std::uppercase;
      }
      os_ << i;
      ++written;
    }
    for (; written < settings_.per_line; ++written) {
      if (written) {
        os_ << ' ';
      }
      os_ << "  ";
    }
    os_ << "  ";
    for (auto&& u : buffer_) {
      if (std::isprint(u)) {
        os_ << char(u);
      } else {
        os_ << '.';
      }
    }
    buffer_.clear();
  }
  using internal_allocator_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<unsigned char>;
  using buffer_type = std::vector<unsigned char,
                                  internal_allocator_type>;
  bool          first_;
  settings_type settings_;
  ostream_type& os_;
  buffer_type   buffer_;
};

/**
 *  \ref basic_hex_dump instantiated with `char` as
 *  its first template argument.
 */
using hex_dump = basic_hex_dump<char>;

}
