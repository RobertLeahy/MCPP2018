mcpp_add_library(serialization
                 SOURCES string.cpp
                         varint.cpp
                 LIBRARIES Boost::boost
                           Boost::system
                           core)
add_subdirectory(tests)
