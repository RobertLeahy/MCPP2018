add_executable(mcpp_core_tests main.cpp)
target_link_libraries(mcpp_core_tests Catch2::Catch2)
add_test(NAME mcpp_core_tests COMMAND mcpp_core_tests)
