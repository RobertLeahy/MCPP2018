mcpp_add_library(yggdrasil
                 SOURCES agent.cpp
                         authenticate.cpp
                         error.cpp
                         profile.cpp
                         property.cpp
                         user.cpp
                 LIBRARIES Boost::boost
                           rapidjson)
add_subdirectory(tests)
