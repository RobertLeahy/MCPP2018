/**
 *  \file
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>
#include "async_result.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/core/noncopyable.hpp>
#include "completion_handler_base.hpp"
#include "hex_dump.hpp"
#include "lowest_layer.hpp"
#include "prefix_buffer_sequence.hpp"
#include "remove_rvalue_reference.hpp"

namespace mcpp {

/**
 *  An object which contains the settings for a
 *  \ref debug_async_stream object.
 *
 *  This is defined at namespace scope rather than
 *  as a nested class as it does not depend on
 *  template parameters to \ref debug_async_stream.
 */
class debug_async_stream_settings : public hex_dump_settings {
public:
  /**
   *  A name which describes the stream being debugged.
   *  If non-empty it shall be written as part of the
   *  debug output.
   */
  std::string name;
  /**
   *  An enumeration of the supported output formats.
   */
  enum class output_format {
    /**
     *  Output will be formatted by a \ref hex_dump
     *  object.
     */
    hex_dump,
    /**
     *  Output will be formatted as text.
     */
    text
  };
  /**
   *  An enumerator indicating the format of output
   *  to generate.
   *
   *  Defaults to \ref output_format::hex_dump.
   */
  output_format format = output_format::hex_dump;
};

/**
 *  A model of `AsyncReadStream` and `AsyncWriteStream`
 *  which layers on top of a type which models
 *  `AsyncReadStream` and/or `AsyncWriteStream` and
 *  passes through all reads and writes before writing
 *  them in a configurable format.
 *
 *  \tparam AsyncStream
 *    The model of `AsyncReadStream` and/or
 *    `AsyncWriteStream` to layer on top of.
 *  \tparam CharT
 *    The character type to use to instantiate
 *    `std::basic_ostream` and \ref hex_dump.
 *  \tparam Traits
 *    The traits type to use to instantiate
 *    `std::basic_ostream` and \ref hex_dump.
 *  \tparam Allocator
 *    The `Allocator` type to use to instantiate
 *    \ref hex_dump.
 */
template<typename AsyncStream,
         typename CharT = char,
         typename Traits = std::char_traits<CharT>,
         typename Allocator = std::allocator<void>>
class debug_async_stream {
public:
  /**
   *  The type of the `AsyncStream` template parameter
   *  after stripping reference qualification.
   */
  using next_layer_type = std::remove_reference_t<AsyncStream>;
  /**
   *  The lowest layer type as computed by
   *  \ref lowest_layer_t.
   */
  using lowest_layer_type = lowest_layer_t<next_layer_type>;
  /**
   *  A type alias for the `CharT` template parameter.
   */
  using char_type = CharT;
  /**
   *  A type alias for the `Traits` template parameter.
   */
  using traits_type = Traits;
  /**
   *  The I/O `Executor` type for `next_layer_type`.
   */
  using executor_type = decltype(std::declval<AsyncStream>().get_executor());
  /**
   *  A type alias for the `Allocator` template parameter.
   */
  using allocator_type = Allocator;
  /**
   *  A type alias for the instantiation of \ref hex_dump
   *  which shall be used.
   */
  using hex_dump_type = basic_hex_dump<char_type,
                                       traits_type,
                                       allocator_type>;
  /**
   *  A type alias for the
   *  \ref debug_async_stream_settings "type used to configure objects of this type".
   */
  using settings_type = debug_async_stream_settings;
  /**
   *  The associated instantiation of `std::basic_ostream`.
   */
  using ostream_type = typename hex_dump_type::ostream_type;
private:
  class state : private boost::noncopyable {
  public:
    template<typename T>
    state(T&& t,
          settings_type settings,
          ostream_type& os,
          const allocator_type& alloc)
      : stream  (std::forward<T>(t)),
        settings(std::move(settings)),
        dumper  (this->settings,
                 os,
                 alloc)
    {}
    AsyncStream   stream;
    settings_type settings;
    hex_dump_type dumper;
  };
  using state_pointer_type = std::shared_ptr<state>;
  template<bool Read,
           typename ConstBufferSequence,
           typename CompletionHandler>
  class op : public completion_handler_base<executor_type,
                                            CompletionHandler>
  {
  private:
    using base = completion_handler_base<executor_type,
                                         CompletionHandler>;
  public:
    template<typename T>
    op(ConstBufferSequence cb,
       T&& t,
       state_pointer_type state)
      : base  (state->stream.get_executor(),
               std::forward<T>(t)),
        cb_   (cb),
        state_(std::move(state))
    {
      assert(state_);
    }
    template<typename ErrorCode>
    void operator()(ErrorCode ec,
                    std::size_t bytes_transferred)
    {
      assert(state_);
      prefix_buffer_sequence pbs(cb_,
                                 bytes_transferred);
      if (!state_->settings.name.empty()) {
        state_->dumper.ostream() << state_->settings.name << ": ";
      }
      if constexpr (Read) {
        state_->dumper.ostream() << "Read";
      } else {
        state_->dumper.ostream() << "Write";
      }
      state_->dumper.ostream() << " (" << bytes_transferred << " bytes)";
      if (bytes_transferred) {
        state_->dumper.ostream() << ":\n";
        if (state_->settings.format == settings_type::output_format::hex_dump) {
          state_->dumper(pbs);
          state_->dumper.done();
        } else {
          for (auto begin = boost::asio::buffer_sequence_begin(pbs), end = boost::asio::buffer_sequence_end(pbs);
               begin != end;
               ++begin)
          {
            boost::asio::const_buffer cb(*begin);
            std::string_view sv(static_cast<const char*>(cb.data()),
                                cb.size());
            state_->dumper.ostream() << sv;
          }
        }
      }
      state_->dumper.ostream() << std::endl;
      base::invoke(std::move(ec),
                   bytes_transferred);
    }
  private:
    ConstBufferSequence cb_;
    state_pointer_type  state_;
  };
public:
  /**
   *  Creates a debug_async_stream.
   *
   *  \tparam T
   *    The type of argument to forward through to
   *    construct the contained `AsyncStream`.
   *
   *  \param [in] t
   *    An object which shall be forwarded through to
   *    a unary constructor of `AsyncStream` (note that
   *    if `AsyncStream` is a reference type this argument
   *    is a reference to which that contained reference
   *    shall be bound).
   *  \param [in] settings
   *    The \ref settings_type "settings object" to use
   *    to configure the newly-constructed object.
   *  \param [in] os
   *    The \ref ostream_type "output stream" to pass
   *    through to a constructor of
   *    \ref hex_dump_type "the contained hex dump object".
   *  \param [in] alloc
   *    The `Allocator` to pass through to a constructor of
   *    \ref hex_dump_type "the contained hex dump object".
   *    Defaults to a default constructed `Allocator`.
   */
  template<typename T>
  debug_async_stream(T&& t,
                     settings_type settings,
                     ostream_type& os,
                     const allocator_type& alloc = allocator_type())
    : state_(std::make_shared<state>(std::forward<T>(t),
                                     std::move(settings),
                                     os,
                                     alloc))
  {}
  /**
   *  Obtains the `Executor` associated with the contained
   *  `AsyncStream`.
   *
   *  \return
   *    An `Executor`.
   */
  executor_type get_executor() noexcept {
    assert(state_);
    return state_->stream.get_executor();
  }
#ifndef MCPP_DOXYGEN_RUNNING
  template<typename ConstBufferSequence,
           typename CompletionToken>
  auto async_write_some(ConstBufferSequence cb,
                        CompletionToken&& t)
  {
    assert(state_);
    using signature_type = void(std::error_code,
                                std::size_t);
    using handler_type = completion_handler_t<CompletionToken,
                                              signature_type>;
    handler_type h(std::forward<CompletionToken>(t));
    using result_type = async_result_t<CompletionToken,
                                       signature_type>;
    result_type result(h);
    using op_type = op<false,
                       ConstBufferSequence,
                       handler_type>;
    state_->stream.async_write_some(cb,
                                    op_type(cb,
                                            std::move(h),
                                            state_));
    return result.get();
  }
  template<typename MutableBufferSequence,
           typename CompletionToken>
  auto async_read_some(MutableBufferSequence mb,
                       CompletionToken&& t)
  {
    assert(state_);
    using signature_type = void(std::error_code,
                                std::size_t);
    using handler_type = completion_handler_t<CompletionToken,
                                              signature_type>;
    handler_type h(std::forward<CompletionToken>(t));
    using result_type = async_result_t<CompletionToken,
                                       signature_type>;
    result_type result(h);
    using op_type = op<true,
                       MutableBufferSequence,
                       handler_type>;
    state_->stream.async_read_some(mb,
                                   op_type(mb,
                                           std::move(h),
                                           state_));
    return result.get();
  }
#endif
  /**
   *  @{
   *  Obtains the \ref next_layer_type "next layer" in
   *  the stack of streams.
   *
   *  \return
   *    A reference to the \ref next_layer_type "next layer".
   */
  next_layer_type& next_layer() noexcept {
    assert(state_);
    return state_->stream;
  }
  const next_layer_type& next_layer() const noexcept {
    assert(state_);
    return state_->stream;
  }
  /**
   *  @}
   *  @{
   *  Obtains the \ref lowest_layer_type "lowest layer" in
   *  the stack of streams.
   *
   *  \return
   *    A reference to the \ref lowest_layer_type "lowest layer".
   */
  lowest_layer_type& lowest_layer() noexcept {
    assert(state_);
    return get_lowest_layer(state_->stream);
  }
  const lowest_layer_type& lowest_layer() const noexcept {
    assert(state_);
    return get_lowest_layer(state_->stream);
  }
  /**
   *  @}
   */
private:
  state_pointer_type state_;
};

template<typename T,
         typename CharT,
         typename Traits>
debug_async_stream(T&&,
                   debug_async_stream_settings,
                   std::basic_ostream<CharT,
                                      Traits>&) -> debug_async_stream<remove_rvalue_reference_t<T>,
                                                                      CharT,
                                                                      Traits>;
template<typename T,
         typename CharT,
         typename Traits,
         typename Allocator>
debug_async_stream(T&&,
                   debug_async_stream_settings,
                   std::basic_ostream<CharT,
                                      Traits>&,
                   const Allocator&) -> debug_async_stream<remove_rvalue_reference_t<T>,
                                                           CharT,
                                                           Traits,
                                                           Allocator>;

}
