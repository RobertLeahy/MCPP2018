add_executable(core_tests any_storage.cpp
                          main.cpp
                          polymorphic_ptr.cpp
                          system_error.cpp)
target_link_libraries(core_tests Boost::boost
                                 Boost::system
                                 Catch2::Catch2
                                 core
                                 testing)
add_test(NAME core_tests COMMAND core_tests)
