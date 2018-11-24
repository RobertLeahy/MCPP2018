mcpp_add_test(serialization
              SOURCES async_read_varint.cpp
                      async_write_varint.cpp
                      endian.cpp
                      hex_digest.cpp
                      main.cpp
                      string.cpp
                      varint.cpp
              LIBRARIES Boost::boost
                        Boost::system
                        Catch2::Catch2
                        core
                        crypto
                        OpenSSL::Crypto
                        testing
                        Threads::Threads)
