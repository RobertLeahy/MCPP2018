/**
 *  \file
 */

#pragma once

#include <cstddef>

namespace mcpp::rapidjson {

template<std::size_t N,
         typename Writer>
bool key(const char (&arr)[N],
         Writer& writer)
{
  return writer.Key(arr,
                    sizeof(arr) - 1U,
                    false);
}

}
