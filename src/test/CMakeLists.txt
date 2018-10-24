mcpp_add_library(testing
                 SOURCES object.cpp
                 LIBRARIES Boost::boost
                           Boost::system
                           core
                 NO_INSTALL)
