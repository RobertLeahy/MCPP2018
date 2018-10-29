mcpp_add_library(nbt
                 SOURCES iterator_sax_writer.cpp
                         ostream_sax_observer.cpp
                         sax_parse.cpp
                 LIBRARIES Boost::boost
                           Boost::system
                           core
                           serialization)
add_subdirectory(tests)
