mcpp_add_test(protocol
              SOURCES async_read.cpp
                      async_write.cpp
                      compress.cpp
                      decompress.cpp
                      main.cpp
              LIBRARIES Boost::boost
                        Catch2::Catch2
                        core
                        protocol
                        testing
                        zlib)
