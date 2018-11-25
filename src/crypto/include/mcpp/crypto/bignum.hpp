/**
 *  \file
 */

#pragma once

#include <mcpp/handle.hpp>
#include <openssl/bn.h>

namespace mcpp::crypto {

namespace detail {

class bignum_policy : public pointer_handle_policy_base<::BIGNUM*> {
public:
  static native_handle_type create();
  static void destroy(native_handle_type) noexcept;
};

}

/**
 *  An instantiation of the \ref mcpp::handle "handle class template"
 *  which manages a `BIGNUM*`.
 */
using bignum = handle<detail::bignum_policy>;

}
