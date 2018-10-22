add_library(serialization varint.cpp)
set_target_properties(serialization PROPERTIES PREFIX "mcpp_")
target_include_directories(serialization PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                                                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(serialization Boost::boost
                                    Boost::system
                                    core)
install(DIRECTORY "include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(TARGETS serialization
        EXPORT McppConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(CMAKE_TESTING_ENABLED)
  add_subdirectory(tests)
endif()
