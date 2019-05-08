mcpp_add_library(yggdrasil
                 SOURCES agent.cpp
                         authenticate.cpp
                         body_reader.cpp
                         body_writer.cpp
                         error.cpp
                         profile.cpp
                         property.cpp
                         user.cpp
                 LIBRARIES Boost::boost
                           Boost::system
                           core
                           rapidjson
                           RapidJSON::RapidJSON)
add_subdirectory(example)
add_subdirectory(tests)
