function(mcpp_add_library target)
  set(OPTIONS NO_INSTALL)
  set(MVA SOURCES LIBRARIES)
  cmake_parse_arguments("ARG" "${OPTIONS}" "" "${MVA}" ${ARGN})
  if(ARG_SOURCES)
    if(ARG_NO_INSTALL)
      add_library(${target} EXCLUDE_FROM_ALL ${ARG_SOURCES})
    else()
      add_library(${target} ${ARG_SOURCES})
    endif()
    if(BUILD_SHARED_LIBS)
      set(PREFIX "${CMAKE_SHARED_LIBRARY_PREFIX}")
    else()
      set(PREFIX "${CMAKE_STATIC_LIBRARY_PREFIX}")
    endif()
    set_target_properties(${target} PROPERTIES PREFIX "${PREFIX}mcpp_")
    target_include_directories(${target} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                                                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    target_link_libraries(${target} ${ARG_LIBRARIES})
    if(NOT ARG_NO_INSTALL)
      install(TARGETS ${target}
              EXPORT McppTargets
              ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
              LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
              RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    endif()
  else()
    if(ARG_NO_INSTALL)
      add_library(${target} INTERFACE EXCLUDE_FROM_ALL)
    else()
      add_library(${target} INTERFACE)
    endif()
    target_include_directories(${target} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                                                   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  endif()
  if(NOT ARG_NO_INSTALL)
    install(DIRECTORY "include/"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
  endif()
endfunction()
function(mcpp_add_test target)
  if(CMAKE_TESTING_ENABLED)
    set(MVA SOURCES LIBRARIES)
    cmake_parse_arguments("ARG" "" "" "${MVA}" ${ARGN})
    set(TEST_TARGET "${target}_tests")
    add_executable(${TEST_TARGET} ${ARG_SOURCES})
    add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})
    target_link_libraries(${TEST_TARGET} ${target} ${ARG_LIBRARIES})
  endif()
endfunction()
