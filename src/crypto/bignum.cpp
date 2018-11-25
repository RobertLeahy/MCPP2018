#include <mcpp/crypto/bignum.hpp>

#include <cassert>
#include <new>
#include <openssl/bn.h>

namespace mcpp::crypto {

namespace detail {

bignum_policy::native_handle_type bignum_policy::create() {
  auto retr = ::BN_new();
  if (!retr) {
    throw std::bad_alloc();
  }
  return retr;
}

void bignum_policy::destroy(native_handle_type handle) noexcept {
  assert(handle);
  ::BN_free(handle);
}

}

}
