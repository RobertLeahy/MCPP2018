mcpp_add_library(crypto
                 SOURCES evp_cipher_ctx.cpp
                         evp_md_ctx.cpp
                 LIBRARIES Boost::boost
                           core
                           OpenSSL::Crypto)
add_subdirectory(tests)
