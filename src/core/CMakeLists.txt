mcpp_add_library(core
                 SOURCES any_storage.cpp
                         system_error.cpp
                         try_copy_n.cpp
                 LIBRARIES Boost::boost
                           Boost::system)
add_subdirectory(tests)
