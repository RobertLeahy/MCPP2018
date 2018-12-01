mcpp_add_test(yggdrasil
              SOURCES agent.cpp
                      authenticate.cpp
                      error.cpp
                      main.cpp
                      profile.cpp
                      property.cpp
                      user.cpp
              LIBRARIES Catch2::Catch2
                        RapidJSON::RapidJSON
                        yggdrasil)
