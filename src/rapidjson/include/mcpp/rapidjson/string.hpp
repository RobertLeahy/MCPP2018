/**
 *  \file
 */

#pragma once

#include <string>
#include <string_view>

namespace mcpp::rapidjson {

template<typename CharT,
         typename Traits,
         typename Writer>
bool string(std::basic_string_view<CharT,
                                   Traits> sv,
            bool copy,
            Writer& writer)
{
  return writer.String(sv.data(),
                       sv.size(),
                       copy);
}
template<typename CharT,
         typename Traits,
         typename Writer>
bool string(std::basic_string_view<CharT,
                                   Traits> sv,
            Writer& writer)
{
  return rapidjson::string(sv,
                           true,
                           writer);
}
template<typename CharT,
         typename Traits,
         typename Allocator,
         typename Writer>
bool string(const std::basic_string<CharT,
                                    Traits,
                                    Allocator>& str,
            bool copy,
            Writer& writer)
{
  std::basic_string_view<CharT,
                         Traits> sv(str);
  return rapidjson::string(sv,
                           copy,
                           writer);
}
template<typename CharT,
         typename Traits,
         typename Allocator,
         typename Writer>
bool string(const std::basic_string<CharT,
                                    Traits,
                                    Allocator>& str,
            Writer& writer)
{
  return rapidjson::string(str,
                           true,
                           writer);
}

}
