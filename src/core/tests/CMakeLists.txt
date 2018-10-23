mcpp_add_test(core
              SOURCES any_storage.cpp
                      main.cpp
                      polymorphic_ptr.cpp
                      system_error.cpp
              LIBRARIES Boost::boost
                        Boost::system
                        Catch2::Catch2
                        testing)
