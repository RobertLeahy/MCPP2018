add_executable(mcpp_core_tests any_storage.cpp
                               main.cpp)
target_link_libraries(mcpp_core_tests Catch2::Catch2
                                      mcpp_core
                                      mcpp_test)
add_test(NAME mcpp_core_tests COMMAND mcpp_core_tests)
