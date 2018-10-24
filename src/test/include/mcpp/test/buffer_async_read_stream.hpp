#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <mcpp/async_result.hpp>

namespace mcpp::test {

template<typename Executor = boost::asio::io_context::executor_type>
class basic_buffer_async_read_stream : private boost::noncopyable {
public:
  using executor_type = Executor;
  explicit basic_buffer_async_read_stream(const executor_type& ex) noexcept
    : ex_  (ex),
      ptr_ (nullptr),
      size_(0),
      read_(0)
  {}
  basic_buffer_async_read_stream(const executor_type& ex,
                                 const std::byte* ptr,
                                 std::size_t size) noexcept
    : ex_  (ex),
      ptr_ (ptr),
      size_(size),
      read_(0)
  {
    assert(ptr_ || !size_);
  }
  executor_type get_executor() noexcept {
    return ex_;
  }
  template<typename MutableBufferSequence,
           typename CompletionToken>
  auto async_read_some(MutableBufferSequence mb,
                       CompletionToken&& t)
  {
    using signature_type = void(boost::system::error_code,
                                std::size_t);
    using completion_handler_type = completion_handler_t<CompletionToken,
                                                         signature_type>;
    completion_handler_type h(std::forward<CompletionToken>(t));
    async_result_t<CompletionToken,
                   signature_type> result(h);
    boost::system::error_code ec;
    std::size_t bytes_transferred = 0;
    if (boost::asio::buffer_size(mb)) {
      bytes_transferred = boost::asio::buffer_copy(mb,
                                                   boost::asio::buffer(ptr_ + read_,
                                                                       size_ - read_));
      read_ += bytes_transferred;
      if (!bytes_transferred) {
        ec = make_error_code(boost::asio::error::eof);
      }
    }
    auto ex = boost::asio::get_associated_executor(h,
                                                   ex_);
    auto bound = boost::asio::bind_executor(ex,
                                            std::bind(std::move(h),
                                                      ec,
                                                      bytes_transferred));
    boost::asio::post(std::move(bound));
    return result.get();
  }
  std::size_t read() const noexcept {
    return read_;
  }
  auto buffer() const noexcept {
    return std::pair(ptr_,
                     size_);
  }
  void buffer(const std::byte* ptr,
              std::size_t size) noexcept
  {
    assert(ptr_ || !size_);
    ptr_ = ptr;
    size_ = size;
  }
private:
  executor_type    ex_;
  const std::byte* ptr_;
  std::size_t      size_;
  std::size_t      read_;
};

using buffer_async_read_stream = basic_buffer_async_read_stream<>;

}
