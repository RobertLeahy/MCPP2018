add_library(core any_storage.cpp
                 system_error.cpp)
set_target_properties(core PROPERTIES PREFIX "mcpp_")
target_include_directories(core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                                       $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(core Boost::boost
                           Boost::system)
install(DIRECTORY "include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(TARGETS core
        EXPORT McppConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(CMAKE_TESTING_ENABLED)
  add_subdirectory(tests)
endif()
