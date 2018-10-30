mcpp_add_test(zlib
              SOURCES deflate.cpp
                      inflate.cpp
                      main.cpp
              LIBRARIES Catch2::Catch2
                        zlib)
