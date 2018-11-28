mcpp_add_test(yggdrasil
              SOURCES agent.cpp
                      error.cpp
                      main.cpp
              LIBRARIES Catch2::Catch2
                        RapidJSON::RapidJSON
                        yggdrasil)
