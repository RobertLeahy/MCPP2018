add_library(mcpp_core any_storage.cpp)
target_include_directories(mcpp_core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                                            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(mcpp_core Boost::boost)
install(DIRECTORY "include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(TARGETS mcpp_core
        EXPORT McppConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(CMAKE_TESTING_ENABLED)
  add_subdirectory(tests)
endif()
