mcpp_add_library(protocol
                 SOURCES async_read.cpp
                 LIBRARIES Boost::boost
                           Boost::system
                           core
                           serialization)
add_subdirectory(tests)
