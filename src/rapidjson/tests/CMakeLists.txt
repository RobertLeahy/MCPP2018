mcpp_add_test(rapidjson
              SOURCES array_parser.cpp
                      bool_parser.cpp
                      const_buffer_sequence_read_stream.cpp
                      iterator_read_stream.cpp
                      state_machine_parser_base.cpp
                      string_parser.cpp
                      main.cpp
                      uint_parser.cpp
              LIBRARIES Boost::boost
                        Catch2::Catch2
                        RapidJSON::RapidJSON)
