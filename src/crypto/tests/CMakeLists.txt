mcpp_add_test(crypto
              SOURCES evp_cipher_ctx.cpp
                      evp_cipher_stream.cpp
                      evp_md_ctx.cpp
                      main.cpp
              LIBRARIES Catch2::Catch2
                        crypto
                        testing)
