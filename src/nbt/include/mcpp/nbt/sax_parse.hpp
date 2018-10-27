/**
 *  \file
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>
#include <mcpp/cast_output_iterator.hpp>
#include <mcpp/serialization/endian.hpp>
#include <mcpp/try_copy_n.hpp>

namespace mcpp::nbt {

namespace detail {

enum class sax_parse_error {
  success = 0,
  eof,
  first_tag_not_compound,
  invalid_tag,
  negative_length
};

std::error_code make_error_code(sax_parse_error) noexcept;

std::error_code sax_parse_check_tag(std::byte) noexcept;

template<typename InputIterator,
         typename>
class sax_parse_compound {
public:
  InputIterator begin;
};
template<typename InputIterator,
         typename>
class sax_parse_list {
public:
  std::byte     type;
  std::int32_t  remaining;
  InputIterator begin;
};
template<typename InputIterator,
         typename Allocator>
using sax_parse_stack_entry = std::variant<sax_parse_compound<InputIterator,
                                                              Allocator>,
                                           sax_parse_list<InputIterator,
                                                          Allocator>>;
template<typename InputIterator,
         typename SaxObserver>
using sax_parse_stack = typename SaxObserver::template vector_type<sax_parse_stack_entry<InputIterator,
                                                                                         typename SaxObserver::allocator_type>>;

template<typename InputIterator>
std::pair<std::byte,
          InputIterator> sax_parse_tag(InputIterator begin,
                                       InputIterator end,
                                       std::error_code& ec)
{
  ec.clear();
  if (begin == end) {
    ec = make_error_code(sax_parse_error::eof);
    return std::pair(std::byte{0},
                     end);
  }
  auto b = *begin;
  ++begin;
  return std::pair(b,
                   begin);
}

template<typename String,
         typename InputIterator>
std::pair<String,
          InputIterator> sax_parse_string(InputIterator begin,
                                          InputIterator end,
                                          String str,
                                          std::error_code& ec)
{
  using char_type = typename String::value_type;
  static_assert(sizeof(char_type) == 1);
  assert(str.empty());
  ec.clear();
  auto pair = serialization::from_endian<std::uint16_t>(begin,
                                                        end,
                                                        ec);
  if (ec) {
    return std::pair(std::move(str),
                     pair.second);
  }
  auto iters = mcpp::try_copy_n(pair.second,
                                end,
                                pair.first,
                                mcpp::make_cast_output_iterator<char_type>(std::back_inserter(str)),
                                ec);
  return std::pair(std::move(str),
                   iters.first);
}

template<typename InputIterator,
         typename SaxObserver>
InputIterator sax_parse_name(InputIterator begin,
                             InputIterator end,
                             SaxObserver& observer,
                             std::error_code& ec)
{
  ec.clear();
  auto pair = detail::sax_parse_string(begin,
                                       end,
                                       typename SaxObserver::string_type(observer.get_allocator()),
                                       ec);
  if (ec) {
    observer.error(ec,
                   pair.second);
    return pair.second;
  }
  ec = observer.name(std::move(pair.first),
                     begin,
                     pair.second);
  return pair.second;
}

template<typename Endian,
         typename InputIterator,
         typename SaxObserver>
InputIterator sax_parse_endian(InputIterator begin,
                               InputIterator end,
                               SaxObserver& observer,
                               std::error_code& ec)
{
  auto pair = serialization::from_endian<Endian>(begin,
                                                 end,
                                                 ec);
  if (ec) {
    observer.error(ec,
                   pair.second);
    return pair.second;
  }
  ec = observer.value(pair.first,
                      begin,
                      pair.second);
  return pair.second;
}

template<typename InputIterator,
         typename SaxObserver>
std::pair<std::int32_t,
          InputIterator> sax_parse_length(InputIterator begin,
                                          InputIterator end,
                                          SaxObserver& observer,
                                          std::error_code& ec)
{
  ec.clear();
  auto pair = serialization::from_endian<std::int32_t>(begin,
                                                       end,
                                                       ec);
  if (ec) {
    observer.error(ec,
                   pair.second);
    return pair;
  }
  if (pair.first < 0) {
    ec = make_error_code(sax_parse_error::negative_length);
    observer.error(ec,
                   begin);
    pair.second = begin;
    return pair;
  }
  ec = observer.length(pair.first,
                       begin,
                       pair.second);
  return pair;
}

template<typename Endian,
         typename InputIterator,
         typename SaxObserver>
InputIterator sax_parse_array(InputIterator begin,
                              InputIterator end,
                              SaxObserver& observer,
                              std::error_code& ec)
{
  auto pair = detail::sax_parse_length(begin,
                                       end,
                                       observer,
                                       ec);
  if (ec) {
    return pair.second;
  }
  assert(pair.first >= 0);
  begin = pair.second;
  for (std::int32_t i = 0; i < pair.first; ++i) {
    auto pair = serialization::from_endian<Endian>(begin,
                                                   end,
                                                   ec);
    if (ec) {
      observer.error(ec,
                     pair.second);
      return pair.second;
    }
    ec = observer.value(pair.first,
                        begin,
                        pair.second);
    if (ec) {
      return pair.second;
    }
    begin = pair.second;
  }
  return begin;
}

template<typename InputIterator,
         typename SaxObserver>
InputIterator sax_parse_value(std::byte tag,
                              InputIterator begin,
                              InputIterator end,
                              sax_parse_stack<InputIterator,
                                              SaxObserver>& stack,
                              SaxObserver& observer,
                              std::error_code& ec)
{
  (void)stack;
  ec.clear();
  switch (std::to_integer<std::uint8_t>(tag)) {
  case 1:{
    if (begin == end) {
      ec = make_error_code(sax_parse_error::eof);
      observer.error(ec,
                     end);
      return end;
    }
    auto i = std::to_integer<std::int8_t>(*begin);
    auto iter = begin;
    ++iter;
    ec = observer.value(i,
                        begin,
                        iter);
    return iter;
  }
  case 2:
    return detail::sax_parse_endian<std::int16_t>(begin,
                                                  end,
                                                  observer,
                                                  ec);
  case 3:
    return detail::sax_parse_endian<std::int32_t>(begin,
                                                  end,
                                                  observer,
                                                  ec);
  case 4:
    return detail::sax_parse_endian<std::int64_t>(begin,
                                                  end,
                                                  observer,
                                                  ec);
  case 5:
    return detail::sax_parse_endian<float>(begin,
                                           end,
                                           observer,
                                           ec);
  case 6:
    return detail::sax_parse_endian<double>(begin,
                                            end,
                                            observer,
                                            ec);
  case 7:{
    ec = observer.begin_byte_array(begin);
    if (ec) {
      return begin;
    }
    begin = detail::sax_parse_array<std::int8_t>(begin,
                                                 end,
                                                 observer,
                                                 ec);
    if (ec) {
      return begin;
    }
    ec = observer.end_byte_array(begin);
    return begin;
  }
  case 8:{
    auto pair = detail::sax_parse_string(begin,
                                         end,
                                         typename SaxObserver::string_type(observer.get_allocator()),
                                         ec);
    if (ec) {
      observer.error(ec,
                     pair.second);
      return pair.second;
    }
    ec = observer.value(std::move(pair.first),
                        begin,
                        pair.second);
    return pair.second;
  }
  case 9:{
    ec = observer.begin_list(begin);
    if (ec) {
      return begin;
    }
    auto tag_pair = detail::sax_parse_tag(begin,
                                          end,
                                          ec);
    if (ec) {
      observer.error(ec,
                     tag_pair.second);
      return tag_pair.second;
    }
    auto length_pair = serialization::from_endian<std::int32_t>(tag_pair.second,
                                                                end,
                                                                ec);
    if (ec) {
      observer.error(ec,
                     length_pair.second);
      return length_pair.second;
    }
    if (length_pair.first > 0) {
      ec = sax_parse_check_tag(tag_pair.first);
      if (ec) {
        observer.error(ec,
                       begin);
        return begin;
      }
    }
    ec = observer.tag(tag_pair.first,
                      begin,
                      tag_pair.second);
    if (ec) {
      return tag_pair.second;
    }
    ec = observer.length(length_pair.first,
                         tag_pair.second,
                         length_pair.second);
    if (ec) {
      return length_pair.second;
    }
    stack.emplace_back(sax_parse_list<InputIterator,
                                      typename SaxObserver::allocator_type>{tag_pair.first,
                                                                            length_pair.first,
                                                                            begin});
    return length_pair.second;
  }
  case 10:{
    ec = observer.begin_compound(begin);
    if (ec) {
      return begin;
    }
    stack.emplace_back(sax_parse_compound<InputIterator,
                                          typename SaxObserver::allocator_type>{begin});
    return begin;
  }
  case 11:{
    ec = observer.begin_int_array(begin);
    if (ec) {
      return begin;
    }
    begin = detail::sax_parse_array<std::int32_t>(begin,
                                                  end,
                                                  observer,
                                                  ec);
    if (ec) {
      return begin;
    }
    ec = observer.end_int_array(begin);
    return begin;
  }
  case 12:{
    ec = observer.begin_long_array(begin);
    if (ec) {
      return begin;
    }
    begin = detail::sax_parse_array<std::int64_t>(begin,
                                                  end,
                                                  observer,
                                                  ec);
    if (ec) {
      return begin;
    }
    ec = observer.end_long_array(begin);
    return begin;
  }
  default:
    assert(false);
    break;
  }
  return begin;
}

template<typename InputIterator,
         typename SaxObserver>
InputIterator sax_parse_named_value(std::byte tag,
                                    InputIterator begin,
                                    InputIterator end,
                                    sax_parse_stack<InputIterator,
                                                    SaxObserver>& stack,
                                    SaxObserver& observer,
                                    std::error_code& ec)
{
  ec.clear();
  begin = detail::sax_parse_name(begin,
                                 end,
                                 observer,
                                 ec);
  if (ec) {
    return begin;
  }
  return detail::sax_parse_value(tag,
                                 begin,
                                 end,
                                 stack,
                                 observer,
                                 ec);
}

}

/**
 *  Parses NBT and produces a series of SAX-like events.
 *
 *  \tparam InputIterator
 *    A model of `InputIterator` which yields `std::byte`
 *    objects when dereferenced.
 *  \tparam SaxObserver
 *    Given:
 *    - `b` an object of type `std::byte`
 *    - `ec` an object of type `std::error_code`
 *    - `i` an object of type `std::int32_t`
 *    - `iter` an object of type `InputIterator`
 *    - `S` a type which is a `SaxObserver`
 *    - `s` an instance of a type which models `SaxObserver`
 *    - `str` an object of type `S::string_type`
 *    - `T` is any type
 *    - `v` an object of any of these types:
 *      - `S::string_type`
 *      - `std::int8_t`
 *      - `std::int16_t`
 *      - `std::int32_t`
 *      - `std::int64_t`
 *      - `float`
 *      - `double`
 *      .
 *    .
 *    Then the following expressions must be valid and
 *    must have the described semantics:
 *    | Expression                          | Type                                                                                                                              | Semantics                                                                                                                                                                                       |
 *    | ----------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
 *    | `S::allocator_type`                 | A type which satisfies the `ProtoAllocator` concept                                                                               | Used to allocate any required memory                                                                                                                                                            |
 *    | `S::string_type`                    | An instantiation of `std::basic_string`                                                                                           | Used to represent string values parsed from NBT                                                                                                                                                 |
 *    | `S::string_type::allocator_type`    | `std::allocator_traits<S::allocator_type>::rebind_alloc<S::string_type::value_type>`                                              | Used to represent string values parsed from NBT                                                                                                                                                 |
 *    | `S::vector_type<T>`                 | A `SequenceContainer` whose `value_type` is `T` and which is constructible given an expression of type `const S::allocator_type&` | Used for any temporary storage required by the parsing process                                                                                                                                  |
 *    | `S::vector_type<T>::allocator_type` | `std::allocator_traits<S::allocator_type>::rebind_alloc<T>`                                                                       | Used for any temporary storage required by the parsing process                                                                                                                                  |
 *    | `s.begin(iter, iter)`               | `std::error_code`                                                                                                                 | Invoked when parsing begins, arguments are begin and end iterators for the input sequence                                                                                                       |
 *    | `s.begin_byte_array(iter)`          | `std::error_code`                                                                                                                 | Invoked when a byte array is found, before the associated length event, the argument is the position in the input sequence where the array begins                                               |
 *    | `s.begin_compound(iter)`            | `std::error_code`                                                                                                                 | Invoked when parsing begins, arguments are begin and end iterators for the input sequence                                                                                                       |
 *    | `s.begin_int_array(iter)`           | `std::error_code`                                                                                                                 | Invoked when an int array is found, before the associated length event, the argument is the position in the input sequence where the array begins                                               |
 *    | `s.begin_list(iter)`                | `std::error_code`                                                                                                                 | Invoked when parsing begins, arguments are begin and end iterators for the input sequence                                                                                                       |
 *    | `s.begin_long_array(iter)`          | `std::error_code`                                                                                                                 | Invoked when a long array is found, before the associated length event, the argument is the position in the input sequence where the array begins                                               |
 *    | `s.end(iter)`                       | `std::error_code`                                                                                                                 | Invoked when parsing completes successfully, argument is one past last `std::byte` consumed                                                                                                     |
 *    | `s.end_byte_array(iter)`            | `std::error_code`                                                                                                                 | Invoked when a byte array ends, argument is one past the last `std::byte` in the representation of the array                                                                                    |
 *    | `s.end_compound(iter)`              | `std::error_code`                                                                                                                 | Invoked when parsing begins, arguments are begin and end iterators for the input sequence                                                                                                       |
 *    | `s.end_int_array(iter)`             | `std::error_code`                                                                                                                 | Invoked when an int array ends, argument is one past the last `std::byte` in the representation of the array                                                                                    |
 *    | `s.end_list(iter)`                  | `std::error_code`                                                                                                                 | Invoked when parsing begins, arguments are begin and end iterators for the input sequence                                                                                                       |
 *    | `s.end_long_array(iter)`            | `std::error_code`                                                                                                                 | Invoked when a long array ends, argument is one past the last `std::byte` in the representation of the array                                                                                    |
 *    | `s.error(ec, iter)`                 |                                                                                                                                   | Invoked when an error occurs in the parsing process, second argument will be returned from this function                                                                                        |
 *    | `s.get_allocator()`                 | `S::allocator_type`                                                                                                               | Must not throw exceptions                                                                                                                                                                       |
 *    | `s.length(i, iter, iter)`           | `std::error_code`                                                                                                                 | Invoked when the length of an array or list is parsed, first argument is the length, second and third are begin and end iterators for range of bytes parsed into the length                     |
 *    | `s.name(str, iter, iter)`           | `std::error_code`                                                                                                                 | Invoked when the name of a named tag is parsed, first argument is the name, second and third are begin and end iterators for the range of `std::byte` objects parsed into the name              |
 *    | `s.tag(b, iter, iter)`              | `std::error_code`                                                                                                                 | Invoked when any tag is parsed, first argument is the tag, second and third are begin and end iterators which give the location of the `std::byte` in the input sequence                        |
 *    | `s.value(v, iter, iter)`            | `std::error_code`                                                                                                                 | Invoked when any single value is parsed (including a string but discluding arrays and lists), first argument is the value, second and third arguments are range of bytes parsed into that value |
 *
 *  \param [in] begin
 *    An iterator to the first `std::byte` to parse.
 *  \param [in] end
 *    An iterator one past the last `std::byte` to
 *    parse.
 *  \param [in] observer
 *    The `SaxObserver` to which all events generated
 *    by the parsing process shall be dispatched.
 *  \param [out] ec
 *    A `std::error_code` which shall receive the result
 *    of the parse operation.
 *
 *  \return
 *    An iterator pointer to one past the last `std::byte`
 *    parsed on success. On failure an iterator pointing to
 *    the point in the input sequence where the error
 *    occurred.
 */
template<typename InputIterator,
         typename SaxObserver>
InputIterator sax_parse(InputIterator begin,
                        InputIterator end,
                        SaxObserver& observer,
                        std::error_code& ec)
{
  ec = observer.begin(begin,
                      end);
  if (ec) {
    return begin;
  }
  auto tag_pair = detail::sax_parse_tag(begin,
                                        end,
                                        ec);
  if (ec) {
    observer.error(ec,
                   tag_pair.second);
    return tag_pair.second;
  }
  if (tag_pair.first != std::byte{10}) {
    ec = detail::sax_parse_check_tag(tag_pair.first);
    if (!ec) {
      ec = make_error_code(detail::sax_parse_error::first_tag_not_compound);
    }
    observer.error(ec,
                   tag_pair.second);
    return begin;
  }
  ec = observer.tag(tag_pair.first,
                    begin,
                    tag_pair.second);
  if (ec) {
    return tag_pair.second;
  }
  auto curr = detail::sax_parse_name(tag_pair.second,
                                     end,
                                     observer,
                                     ec);
  if (ec) {
    return curr;
  }
  ec = observer.begin_compound(curr);
  if (ec) {
    return curr;
  }
  detail::sax_parse_stack<InputIterator,
                          SaxObserver> stack(observer.get_allocator());
  using allocator_type = typename SaxObserver::allocator_type;
  using compound_type = detail::sax_parse_compound<InputIterator,
                                                   allocator_type>;
  stack.emplace_back(compound_type{curr});
  do {
    assert(!stack.empty());
    auto v = [&](auto&& state) {
      if constexpr (std::is_same_v<std::decay_t<decltype(state)>,
                                   compound_type>)
      {
        auto tag_pair = detail::sax_parse_tag(curr,
                                              end,
                                              ec);
        if (ec) {
          observer.error(ec,
                         tag_pair.second);
          return false;
        }
        if (tag_pair.first != std::byte{0}) {
          ec = detail::sax_parse_check_tag(tag_pair.first);
          if (ec) {
            observer.error(ec,
                           curr);
            return false;
          }
        }
        ec = observer.tag(tag_pair.first,
                          curr,
                          tag_pair.second);
        if (ec) {
          return false;
        }
        curr = tag_pair.second;
        if (tag_pair.first == std::byte{0}) {
          ec = observer.end_compound(curr);
          return !ec;
        }
        curr = detail::sax_parse_named_value(tag_pair.first,
                                             curr,
                                             end,
                                             stack,
                                             observer,
                                             ec);
        if (ec) {
          return false;
        }
      } else {
        if (!state.remaining) {
          ec = observer.end_list(curr);
          return !ec;
        }
        curr = detail::sax_parse_value(state.type,
                                       curr,
                                       end,
                                       stack,
                                       observer,
                                       ec);
        if (ec) {
          return false;
        }
        --state.remaining;
      }
      return false;
    };
    if (std::visit(v,
                   stack.back()))
    {
      stack.pop_back();
    }
    if (ec) {
      return curr;
    }
  } while (!stack.empty());
  observer.end(curr);
  return curr;
}

}
