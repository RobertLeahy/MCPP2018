mcpp_add_test(nbt
              SOURCES main.cpp
                      sax_parse.cpp
              LIBRARIES Boost::boost
                        Catch2::Catch2
                        core
                        nbt
                        nbt_testing
                        ZLIB::ZLIB)
if(TARGET nbt_tests)
  set(BIGTEST_PATH "${CMAKE_CURRENT_BINARY_DIR}/bigtest.nbt")
  file(DOWNLOAD "https://raw.github.com/Dav1dde/nbd/master/test/bigtest.nbt" ${BIGTEST_PATH}
       SHOW_PROGRESS
       EXPECTED_HASH MD5=0027755ca65655ffbb908eafb3f8de8d
       TLS_VERIFY ON)
  target_compile_definitions(nbt_tests PRIVATE
                             MCPP_BIGTEST_PATH="${BIGTEST_PATH}")
endif()
