mcpp_add_library(rapidjson
                 SOURCES bool_parser.cpp
                         done_parser_base.cpp
                         parser_base.cpp
                         uint_parser.cpp
                 LIBRARIES Boost::boost)
add_subdirectory(tests)
