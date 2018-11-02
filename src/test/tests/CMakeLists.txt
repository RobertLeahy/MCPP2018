mcpp_add_test(testing
              SOURCES buffer_async_read_stream.cpp
                      main.cpp
              LIBRARIES Boost::boost
                        Boost::system
                        Catch2::Catch2
                        core
                        testing)
