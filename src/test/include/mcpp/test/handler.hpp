#pragma once

#include <cstddef>
#include <system_error>
#include <boost/core/noncopyable.hpp>
#include <boost/system/error_code.hpp>

namespace mcpp::test {

class handler_state : private boost::noncopyable {
public:
  handler_state() noexcept;
  void clear() noexcept;
  bool invoked;
};

class handler {
public:
  explicit handler(handler_state& state) noexcept;
  void operator()();
private:
  handler_state* state_;
};

class completion_handler_state : public handler_state {
public:
  void clear() noexcept;
  std::error_code ec;
};

class completion_handler : private handler {
public:
  explicit completion_handler(completion_handler_state& state) noexcept;
  void operator()(boost::system::error_code ec);
private:
  completion_handler_state* state_;
};

class read_handler_state : public completion_handler_state {
public:
  read_handler_state() noexcept;
  void clear() noexcept;
  std::size_t bytes_transferred;
};

class read_handler : private completion_handler {
public:
  explicit read_handler(read_handler_state& state) noexcept;
  void operator()(boost::system::error_code ec,
                  std::size_t bytes_transferred);
private:
  read_handler_state* state_;
};

using write_handler_state = read_handler_state;

using write_handler = read_handler;

}
