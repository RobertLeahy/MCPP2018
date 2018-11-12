mcpp_add_library(protocol
                 SOURCES async_read.cpp
                         compress.cpp
                         decompress.cpp
                 LIBRARIES Boost::boost
                           Boost::system
                           core
                           serialization
                           zlib)
add_subdirectory(tests)
