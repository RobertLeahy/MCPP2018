/**
 *  \file
 */

#pragma once

#include <boost/asio/buffers_iterator.hpp>
#include "iterator_read_stream.hpp"

namespace mcpp::rapidjson {

/**
 *  A model of the `Stream` concept from RapidJSON
 *  which reads `char` objects from a sequence of
 *  buffers.
 *
 *  \tparam ConstBufferSequence
 *    A type which models `ConstBufferSequence`.
 */
template<typename ConstBufferSequence>
class const_buffer_sequence_read_stream
#ifndef MCPP_DOXYGEN_RUNNING
: public iterator_read_stream<boost::asio::buffers_iterator<ConstBufferSequence,
                                                            char>>
#endif
{
private:
  using buffers_iterator_type = boost::asio::buffers_iterator<ConstBufferSequence,
                                                              char>;
  using base = iterator_read_stream<buffers_iterator_type>;
public:
  /**
   *  Creates a new const_buffer_sequence_read_stream
   *  which reads from a `ConstBufferSequence`.
   *
   *  \param [in] cb
   *    The sequence of buffers from which to read.
   */
  explicit const_buffer_sequence_read_stream(ConstBufferSequence cb) noexcept
    : base(buffers_iterator_type::begin(cb),
           buffers_iterator_type::end(cb))
  {}
};

}
