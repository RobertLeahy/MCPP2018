/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>

namespace mcpp {

/**
 *  A `ConstBufferSequence` or `MutableBufferSequence`
 *  (depending on its template parameter) which is a
 *  suffix of another `ConstBufferSequence` or
 *  `MutableBufferSequence` (i.e. an object of this type
 *  is a sequence of buffers which contain the last N
 *  bytes of another sequence of buffers).
 *
 *  \tparam BufferSequence
 *    A `ConstBufferSequence` or `MutableBufferSequence`
 *    over which the suffix shall be defined. If this is
 *    a `ConstBufferSequence` then the resulting instantiation
 *    of this class template is a `ConstBufferSequence`, likewise
 *    for `MutableBufferSequence`.
 */
template<typename BufferSequence>
class suffix_buffer_sequence {
private:
  using inner_iterator_type = decltype(boost::asio::buffer_sequence_begin(std::declval<BufferSequence&>()));
  using inner_iterator_traits_type = std::iterator_traits<inner_iterator_type>;
  using buffer_type = std::conditional_t<boost::asio::is_mutable_buffer_sequence<BufferSequence>::value,
                                         boost::asio::mutable_buffer,
                                         boost::asio::const_buffer>;
public:
  /**
   *  Creates a suffix_buffer_sequence.
   *
   *  @param [in] bs
   *    The `ConstBufferSequence` or `MutableBufferSequence`
   *    to represent a suffix of.
   *  @param [in] suffix
   *    The size of the suffix. Must be less than or equal to
   *    `boost::asio::buffer_size(bs)` or the behavior is undefined.
   */
  suffix_buffer_sequence(BufferSequence bs,
                         std::size_t suffix) noexcept(std::is_nothrow_move_constructible_v<BufferSequence>)
    : inner_ (std::move(bs)),
      suffix_(suffix)
  {
    assert(boost::asio::buffer_size(inner_) >= suffix_);
  }
#ifndef MCPP_DOXYGEN_RUNNING
  class iterator {
  public:
    using difference_type = typename inner_iterator_traits_type::difference_type;
    using value_type = buffer_type;
    using pointer = value_type*;
    using reference = value_type;
    using iterator_category = std::bidirectional_iterator_tag;
    iterator() noexcept(std::is_default_constructible_v<inner_iterator_type>)
      : remaining_(0),
        suffix_   (0)
    {}
    iterator(inner_iterator_type inner,
             std::size_t remaining,
             std::size_t suffix) noexcept(std::is_nothrow_move_constructible_v<inner_iterator_type>)
      : inner_    (std::move(inner)),
        remaining_(remaining),
        suffix_   (suffix)
    {}
    reference operator*() const noexcept {
      buffer_type buffer(*inner_);
      if (remaining_ > suffix_) {
        std::size_t remove = remaining_ - suffix_;
        assert(buffer.size() >= remove);
        buffer += remove;
      }
      return buffer;
    }
    iterator& operator++() noexcept(noexcept(++std::declval<inner_iterator_type&>())) {
      buffer_type buffer(*inner_);
      assert(remaining_ >= buffer.size());
      remaining_ -= buffer.size();
      ++inner_;
      return *this;
    }
    iterator& operator--() noexcept(noexcept(--std::declval<inner_iterator_type&>())) {
      --inner_;
      buffer_type buffer(*inner_);
      remaining_ += buffer.size();
      return *this;
    }
    iterator operator++(int) noexcept(noexcept(++std::declval<iterator&>())                     &&
                                      std::is_nothrow_copy_constructible_v<inner_iterator_type> &&
                                      std::is_nothrow_move_constructible_v<inner_iterator_type>)
    {
      iterator retr(*this);
      ++*this;
      return retr;
    }
    iterator operator--(int) noexcept(noexcept(--std::declval<iterator&>())                     &&
                                      std::is_nothrow_copy_constructible_v<inner_iterator_type> &&
                                      std::is_nothrow_move_constructible_v<inner_iterator_type>)
    {
      iterator retr(*this);
      --*this;
      return retr;
    }
    bool operator==(const iterator& rhs) const noexcept(noexcept(std::declval<const inner_iterator_type&>() == std::declval<const inner_iterator_type&>())) {
      return inner_ == rhs.inner_;
    }
    bool operator!=(const iterator& rhs) const noexcept(noexcept(std::declval<const inner_iterator_type&>() == std::declval<const inner_iterator_type&>())) {
      return !(*this == rhs);
    }
  private:
    inner_iterator_type inner_;
    std::size_t         remaining_;
    std::size_t         suffix_;
  };
  iterator begin() const noexcept(noexcept(boost::asio::buffer_sequence_begin(std::declval<const BufferSequence&>())) &&
                                  std::is_nothrow_move_constructible_v<inner_iterator_type>                           &&
                                  noexcept(++std::declval<inner_iterator_type&>())                                    &&
                                  noexcept(boost::asio::buffer_size(std::declval<const BufferSequence&>())))
  {
    std::size_t size = boost::asio::buffer_size(inner_);
    assert(size >= suffix_);
    auto iter = boost::asio::buffer_sequence_begin(inner_);
    for (; size != suffix_; ++iter) {
      std::size_t after = size;
      buffer_type buffer(*iter);
      assert(buffer.size() >= after);
      after -= buffer.size();
      if (after < suffix_) {
        break;
      }
      size = after;
    }
    return iterator(iter,
                    size,
                    suffix_);
  }
  iterator end() const noexcept(noexcept(boost::asio::buffer_sequence_end(std::declval<const BufferSequence&>())) &&
                                std::is_nothrow_move_constructible_v<inner_iterator_type>)
  {
    return iterator(boost::asio::buffer_sequence_end(inner_),
                    0,
                    suffix_);
  }
#endif
private:
  BufferSequence inner_;
  std::size_t    suffix_;
};

}

#ifndef MCPP_DOXYGEN_RUNNING
namespace boost {
namespace asio {

namespace detail {

//  Code using boost::asio::buffers_iterator over
//  suffix_buffer_sequence objects won't compile
//  unless this is provided, which isn't documented
//  anywhere (I expect this just worked before
//  Boost.Asio tried to be like the Networking TS and
//  got lost/overlooked in translation)
template<typename BufferSequence,
         typename ByteType>
struct buffers_iterator_types<mcpp::suffix_buffer_sequence<BufferSequence>,
                              ByteType>
{
public:
  using buffer_type = std::conditional_t<is_mutable_buffer_sequence<BufferSequence>::value,
                                         boost::asio::mutable_buffer,
                                         boost::asio::const_buffer>;
  using byte_type = std::conditional_t<is_mutable_buffer_sequence<BufferSequence>::value,
                                       ByteType,
                                       const ByteType>;
  using const_iterator = typename mcpp::suffix_buffer_sequence<BufferSequence>::iterator;
};

}

//  This should be detected automatically, but to
//  the degree to which it is, it isn't done correctly
//  (it still looks for member functions begin and end
//  which aren't necessary under the Networking TS-compatible
//  formulation)
template<typename BufferSequence>
struct is_mutable_buffer_sequence<mcpp::suffix_buffer_sequence<BufferSequence>> : public boost::asio::is_mutable_buffer_sequence<BufferSequence> {};
template<typename BufferSequence>
struct is_const_buffer_sequence<mcpp::suffix_buffer_sequence<BufferSequence>> : public boost::asio::is_const_buffer_sequence<BufferSequence> {};

template<typename BufferSequence>
auto buffer_sequence_begin(const mcpp::suffix_buffer_sequence<BufferSequence>& sbs) noexcept(noexcept(sbs.begin())) {
  return sbs.begin();
}

template<typename BufferSequence>
auto buffer_sequence_end(const mcpp::suffix_buffer_sequence<BufferSequence>& sbs) noexcept(noexcept(sbs.end())) {
  return sbs.end();
}

}
}
#endif
