#include <mcpp/crypto/rsa.hpp>

#include <cassert>
#include <new>

namespace mcpp::crypto {

rsa::rsa()
  : handle_(::RSA_new())
{
  if (!handle_) {
    throw std::bad_alloc();
  }
}

rsa::rsa(native_handle_type handle) noexcept
  : handle_(handle)
{
  assert(handle_);
}

rsa::rsa(rsa&& other) noexcept
  : handle_(other.handle_)
{
  other.handle_ = nullptr;
}

rsa& rsa::operator=(rsa&& rhs) noexcept {
  assert(this != &rhs);
  destroy();
  handle_ = rhs.handle_;
  rhs.handle_ = nullptr;
  return *this;
}

rsa::~rsa() noexcept {
  destroy();
}

rsa::native_handle_type rsa::native_handle() noexcept {
  assert(handle_);
  return handle_;
}

void rsa::destroy() noexcept {
  if (handle_) {
    ::RSA_free(handle_);
  }
}

}
