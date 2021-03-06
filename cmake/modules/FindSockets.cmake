add_library(Sockets::Sockets INTERFACE IMPORTED)
if (${CMAKE_SYSTEM} MATCHES "Windows")
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		target_link_libraries(Sockets::Sockets ws2_32)
  else()
		target_link_libraries(Sockets::Sockets wsock32 ws2_32)
  endif()
elseif(${CMAKE_SYSTEM} MATCHES "INtime")
	target_link_libraries(Sockets::Sockets netlib)
endif()
