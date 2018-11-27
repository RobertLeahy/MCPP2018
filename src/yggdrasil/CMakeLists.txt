mcpp_add_library(yggdrasil
                 SOURCES error.cpp
                 LIBRARIES Boost::boost
                           rapidjson)
add_subdirectory(tests)
