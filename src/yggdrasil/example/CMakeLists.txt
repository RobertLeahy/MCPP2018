add_executable(yggdrasil_example example.cpp)
target_link_libraries(yggdrasil_example
                      Boost::boost
                      Boost::system
                      core
                      OpenSSL::SSL
                      Threads::Threads
                      yggdrasil)
