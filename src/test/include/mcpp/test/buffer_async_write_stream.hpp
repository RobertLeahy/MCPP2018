#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <utility>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <mcpp/async_result.hpp>

namespace mcpp::test {

template<typename Executor = boost::asio::io_context::executor_type>
class basic_buffer_async_write_stream : private boost::noncopyable {
public:
  using executor_type = Executor;
  explicit basic_buffer_async_write_stream(const executor_type& ex) noexcept
    : ex_     (ex),
      ptr_    (nullptr),
      size_   (0),
      written_(0)
  {}
  basic_buffer_async_write_stream(const executor_type& ex,
                                  std::byte* ptr,
                                  std::size_t size) noexcept
    : ex_     (ex),
      ptr_    (ptr),
      size_   (size),
      written_(0)
  {
    assert(ptr_ || !size_);
  }
  template<typename ConstBufferSequence,
           typename CompletionToken>
  auto async_write_some(ConstBufferSequence cb,
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
    if (boost::asio::buffer_size(cb)) {
      assert(size_ >= written_);
      bytes_transferred = std::min(size_ - written_,
                                   boost::asio::buffer_size(cb));
      std::copy_n(boost::asio::buffers_iterator<ConstBufferSequence,
                                                std::byte>::begin(cb),
                  bytes_transferred,
                  ptr_ + written_);
      written_ += bytes_transferred;
      assert(size_ >= written_);
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
  executor_type get_executor() noexcept {
    return ex_;
  }
  std::size_t written() const noexcept {
    return written_;
  }
  auto buffer() const noexcept {
    return std::pair(ptr_,
                     size_);
  }
  void buffer(std::byte* ptr,
              std::size_t size) noexcept
  {
    assert(ptr_ || !size_);
    ptr_ = ptr;
    size_ = size;
    written_ = 0;
  }
private:
  executor_type ex_;
  std::byte*    ptr_;
  std::size_t   size_;
  std::size_t   written_;
};

using buffer_async_write_stream = basic_buffer_async_write_stream<>;

}
