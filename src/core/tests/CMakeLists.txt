mcpp_add_test(core
              SOURCES allocate_unique.cpp
                      any_storage.cpp
                      async_result.cpp
                      cast_output_iterator.cpp
                      checked.cpp
                      completion_handler_base.cpp
                      handle.cpp
                      hex_dump.cpp
                      lowest_layer.cpp
                      main.cpp
                      polymorphic_ptr.cpp
                      prefix_buffer_sequence.cpp
                      remove_optional.cpp
                      remove_rvalue_reference.cpp
                      suffix_buffer_sequence.cpp
                      system_error.cpp
                      try_copy_n.cpp
              LIBRARIES Boost::boost
                        Boost::system
                        Catch2::Catch2
                        testing
                        Threads::Threads)
