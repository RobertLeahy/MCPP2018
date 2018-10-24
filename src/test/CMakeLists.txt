mcpp_add_library(testing
                 SOURCES allocator.cpp
                         handler.cpp
                         object.cpp
                 LIBRARIES Boost::boost
                           Boost::system
                           core
                           Threads::Threads
                 NO_INSTALL)
add_subdirectory(tests)
