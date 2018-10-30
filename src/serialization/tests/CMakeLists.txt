mcpp_add_test(serialization
              SOURCES async_read_varint.cpp
                      async_write_varint.cpp
                      endian.cpp
                      main.cpp
                      string.cpp
                      varint.cpp
              LIBRARIES Boost::boost
                        Boost::system
                        Catch2::Catch2
                        core
                        testing
                        Threads::Threads)
