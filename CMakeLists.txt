cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(mcpp LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 1)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_definitions(-DBOOST_ALL_NO_LIB)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")
find_package(Boost 1.68.0 REQUIRED system)
find_package(Catch2)
find_package(Doxygen)
find_package(Sockets REQUIRED)
find_package(Threads REQUIRED)
include(GNUInstallDirs)
if(Catch2_FOUND)
  enable_testing()
endif()
add_subdirectory(src)
if(DOXYGEN_FOUND)
  if(DOXYGEN_DOT_FOUND)
    set(DOXYGEN_CONFIGURE_HAVE_DOT "HAVE_DOT=YES")
    get_filename_component(DOXYGEN_DOT_PATH "${DOXYGEN_DOT_EXECUTABLE}" DIRECTORY)
    file(TO_NATIVE_PATH "${DOXYGEN_DOT_PATH}" DOXYGEN_DOT_PATH)
    set(DOXYGEN_CONFIGURE_DOT_PATH "DOT_PATH=${DOXYGEN_DOT_PATH}")
  else()
    set(DOXYGEN_CONFIGURE_HAVE_DOT "HAVE_DOT=NO")
  endif()
  configure_file(Doxyfile.in "${CMAKE_BINARY_DIR}/Doxyfile")
  add_custom_target(doc
                    ${DOXYGEN_EXECUTABLE}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Running Doxygen" VERBATIM)
  install(CODE "execute_process(COMMAND ${DOXYGEN_EXECUTABLE} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})")
  install(DIRECTORY "${CMAKE_BINARY_DIR}/doc/html/"
          DESTINATION "${CMAKE_INSTALL_DOCDIR}")
endif()
install(EXPORT McppConfig DESTINATION share/Mcpp/cmake)
