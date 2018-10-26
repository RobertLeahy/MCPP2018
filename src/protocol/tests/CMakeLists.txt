mcpp_add_test(protocol
              SOURCES async_read.cpp
                      main.cpp
              LIBRARIES Boost::boost
                        Catch2::Catch2
                        core
                        protocol
                        testing)
