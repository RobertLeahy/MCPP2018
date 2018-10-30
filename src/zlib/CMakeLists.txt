mcpp_add_library(zlib
                 SOURCES deflate.cpp
                         inflate.cpp
                         stream.cpp
                         system_error.cpp
                 LIBRARIES Boost::boost
                           Boost::system
                           core
                           ZLIB::ZLIB)
add_subdirectory(tests)
