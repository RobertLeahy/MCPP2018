mcpp_add_test(yggdrasil
              SOURCES error.cpp
                      main.cpp
              LIBRARIES Catch2::Catch2
                        RapidJSON::RapidJSON
                        yggdrasil)
