#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

namespace mcpp::nbt::test {

template<typename InputIterator,
         typename CharT = char,
         typename Traits = std::char_traits<CharT>,
         typename Allocator = std::allocator<void>>
class basic_sax_observer {
public:
  using char_type = CharT;
  using traits_type = Traits;
  using allocator_type = Allocator;
  using iterator = InputIterator;
private:
  template<typename T>
  using allocator_for_t = typename std::allocator_traits<allocator_type>::template rebind_alloc<T>;
public:
  template<typename T>
  using vector_type = std::vector<T,
                                  allocator_for_t<T>>;
  using string_type = std::basic_string<char_type,
                                        traits_type,
                                        allocator_for_t<char_type>>;
  class begin_event {
  public:
    iterator begin;
    iterator end;
  };
  class end_event {
  public:
    iterator where;
  };
  class error_event {
  public:
    std::error_code ec;
    iterator        where;
  };
  class tag_event {
  public:
    std::byte tag;
    iterator  begin;
    iterator  end;
  };
  class name_event {
  public:
    string_type name;
    iterator    begin;
    iterator    end;
  };
  class begin_compound_event {
  public:
    iterator where;
  };
  class end_compound_event {
  public:
    iterator where;
  };
  template<typename Value>
  class value_event {
  public:
    Value    value;
    iterator begin;
    iterator end;
  };
  using string_event = value_event<string_type>;
  using byte_event = value_event<std::int8_t>;
  using short_event = value_event<std::int16_t>;
  using int_event = value_event<std::int32_t>;
  using long_event = value_event<std::int64_t>;
  using float_event = value_event<float>;
  using double_event = value_event<double>;
  class begin_list_event {
  public:
    iterator begin;
  };
  class end_list_event {
  public:
    iterator begin;
  };
  class length_event {
  public:
    std::int32_t length;
    iterator     begin;
    iterator     end;
  };
  template<typename T>
  class begin_array_event {
  public:
    iterator where;
  };
  using begin_byte_array_event = begin_array_event<std::int8_t>;
  using begin_int_array_event = begin_array_event<std::int32_t>;
  using begin_long_array_event = begin_array_event<std::int64_t>;
  template<typename T>
  class end_array_event {
  public:
    iterator where;
  };
  using end_byte_array_event = end_array_event<std::int8_t>;
  using end_int_array_event = end_array_event<std::int32_t>;
  using end_long_array_event = end_array_event<std::int64_t>;
  using event_type = std::variant<begin_event,
                                  error_event,
                                  end_event,
                                  tag_event,
                                  name_event,
                                  begin_compound_event,
                                  end_compound_event,
                                  string_event,
                                  byte_event,
                                  short_event,
                                  int_event,
                                  long_event,
                                  float_event,
                                  double_event,
                                  begin_list_event,
                                  end_list_event,
                                  length_event,
                                  begin_byte_array_event,
                                  end_byte_array_event,
                                  begin_int_array_event,
                                  end_int_array_event,
                                  begin_long_array_event,
                                  end_long_array_event>;
  explicit basic_sax_observer(const allocator_type& alloc = allocator_type())
    : alloc_(alloc)
  {}
  allocator_type get_allocator() const noexcept {
    return alloc_;
  }
  using events_type = std::vector<event_type>;
  events_type events;
  std::error_code begin(iterator begin,
                        iterator end)
  {
    events.emplace_back(begin_event{begin,
                                    end});
    return std::error_code();
  }
  void error(std::error_code ec,
             iterator where)
  {
    events.emplace_back(error_event{ec,
                                    where});
  }
  std::error_code end(iterator where) {
    events.emplace_back(end_event{where});
    return std::error_code();
  }
  std::error_code tag(std::byte tag,
                      iterator begin,
                      iterator end)
  {
    events.emplace_back(tag_event{tag,
                                  begin,
                                  end});
    return std::error_code();
  }
  std::error_code name(string_type str,
                       iterator begin,
                       iterator end)
  {
    events.emplace_back(name_event{std::move(str),
                                   begin,
                                   end});
    return std::error_code();
  }
  std::error_code begin_compound(iterator where) {
    events.emplace_back(begin_compound_event{where});
    return std::error_code();
  }
  std::error_code end_compound(iterator where) {
    events.emplace_back(end_compound_event{where});
    return std::error_code();
  }
  template<typename Value>
  std::error_code value(Value value,
                        iterator begin,
                        iterator end)
  {
    events.emplace_back(value_event<Value>{std::move(value),
                                           begin,
                                           end});
    return std::error_code();
  }
  std::error_code begin_list(iterator where) {
    events.emplace_back(begin_list_event{where});
    return std::error_code();
  }
  std::error_code end_list(iterator where) {
    events.emplace_back(end_list_event{where});
    return std::error_code();
  }
  std::error_code length(std::int32_t length,
                         iterator begin,
                         iterator end)
  {
    events.emplace_back(length_event{length,
                                     begin,
                                     end});
    return std::error_code();
  }
  std::error_code begin_byte_array(iterator where) {
    events.emplace_back(begin_byte_array_event{where});
    return std::error_code();
  }
  std::error_code end_byte_array(iterator where) {
    events.emplace_back(end_byte_array_event{where});
    return std::error_code();
  }
  std::error_code begin_int_array(iterator where) {
    events.emplace_back(begin_int_array_event{where});
    return std::error_code();
  }
  std::error_code end_int_array(iterator where) {
    events.emplace_back(end_int_array_event{where});
    return std::error_code();
  }
  std::error_code begin_long_array(iterator where) {
    events.emplace_back(begin_long_array_event{where});
    return std::error_code();
  }
  std::error_code end_long_array(iterator where) {
    events.emplace_back(end_long_array_event{where});
    return std::error_code();
  }
private:
  allocator_type alloc_;
};

using sax_observer = basic_sax_observer<std::vector<std::byte>::const_iterator>;

}
