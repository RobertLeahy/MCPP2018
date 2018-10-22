add_executable(serialization_tests main.cpp
                                   varint.cpp)
target_link_libraries(serialization_tests Boost::boost
                                          Boost::system
                                          Catch2::Catch2
                                          core
                                          serialization)
add_test(NAME serialization_tests COMMAND serialization_tests)
