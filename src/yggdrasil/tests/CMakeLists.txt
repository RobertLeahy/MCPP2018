mcpp_add_test(yggdrasil
              SOURCES agent.cpp
                      authenticate.cpp
                      body_reader.cpp
                      body_writer.cpp
                      error.cpp
                      main.cpp
                      profile.cpp
                      property.cpp
                      user.cpp
              LIBRARIES Boost::boost
                        Boost::system
                        Catch2::Catch2
                        RapidJSON::RapidJSON
                        testing
                        yggdrasil)
