mcpp_add_test(serialization
              SOURCES endian.cpp
                      main.cpp
                      string.cpp
                      varint.cpp
              LIBRARIES Boost::boost
                        Boost::system
                        Catch2::Catch2
                        core)
