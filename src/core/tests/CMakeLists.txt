mcpp_add_test(core
              SOURCES any_storage.cpp
                      async_result.cpp
                      cast_output_iterator.cpp
                      main.cpp
                      polymorphic_ptr.cpp
                      system_error.cpp
                      try_copy_n.cpp
              LIBRARIES Boost::boost
                        Boost::system
                        Catch2::Catch2
                        testing
                        Threads::Threads)
