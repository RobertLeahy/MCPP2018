mcpp_add_library(crypto
                 SOURCES bignum.cpp
                         evp_cipher_ctx.cpp
                         evp_md_ctx.cpp
                         rsa.cpp
                         system_error.cpp
                 LIBRARIES Boost::boost
                           core
                           OpenSSL::Crypto)
add_subdirectory(tests)
