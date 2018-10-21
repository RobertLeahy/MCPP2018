add_library(mcpp_test object.cpp)
target_link_libraries(mcpp_test Boost::boost)
target_include_directories(mcpp_test PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
