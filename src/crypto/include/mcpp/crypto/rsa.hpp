/**
 *  \file
 */

#pragma once

#include <mcpp/handle.hpp>
#include <openssl/rsa.h>

namespace mcpp::crypto {

namespace detail {

class rsa_policy : public pointer_handle_policy_base<::RSA*> {
public:
  static native_handle_type create();
  static void destroy(native_handle_type) noexcept;
};

}

/**
 *  An instantiation of the \ref mcpp::handle "handle class template"
 *  which manages an `RSA*`.
 */
using rsa = handle<detail::rsa_policy>;

}
