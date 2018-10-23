mcpp_add_library(core
                 SOURCES any_storage.cpp
                         system_error.cpp
                 LIBRARIES Boost::boost
                           Boost::system)
add_subdirectory(tests)
