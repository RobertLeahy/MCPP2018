add_library(testing object.cpp)
target_link_libraries(testing Boost::boost)
target_include_directories(testing PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
