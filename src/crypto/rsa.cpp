#include <mcpp/crypto/rsa.hpp>

#include <cassert>
#include <new>

namespace mcpp::crypto {

namespace detail {

rsa_policy::native_handle_type rsa_policy::create() {
  native_handle_type retr = ::RSA_new();
  if (!retr) {
    throw std::bad_alloc();
  }
  return retr;
}

void rsa_policy::destroy(native_handle_type handle) noexcept {
  assert(handle);
  ::RSA_free(handle);
}

}

}
