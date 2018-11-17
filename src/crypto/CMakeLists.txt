mcpp_add_library(crypto
                 SOURCES evp_cipher_ctx.cpp
                 LIBRARIES Boost::boost
                           OpenSSL::Crypto)
add_subdirectory(tests)
