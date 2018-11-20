mcpp_add_test(crypto
              SOURCES evp_cipher_ctx.cpp
                      evp_cipher_stream.cpp
                      main.cpp
              LIBRARIES Catch2::Catch2
                        crypto
                        testing)
