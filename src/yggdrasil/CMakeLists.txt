mcpp_add_library(yggdrasil
                 SOURCES agent.cpp
                         error.cpp
                 LIBRARIES Boost::boost
                           rapidjson)
add_subdirectory(tests)
