/**
 *  \file
 */

#pragma once

#include <cassert>
#include <type_traits>
#include <utility>
#include <variant>
#include "parser_base.hpp"

namespace mcpp::rapidjson {

/**
 *  A convenience base class for implementing
 *  parsers using a state machine.
 *
 *  \tparam Parsers
 *    The complete set of parsers in the state
 *    machine diagram.
 */
template<typename... Parsers>
class state_machine_parser_base : public parser_base {
public:
  /**
   *  Destroys the contained parser (if any) thereby
   *  resetting this object.
   */
  void clear() noexcept {
    parser_.template emplace<std::monostate>();
  }
  /**
   *  Determines whether this object contains a
   *  parser. Note that after each SAX event is
   *  received by this object and dispatched to
   *  the contained parser it checks to see if
   *  that parser is done, and if it is destroys
   *  it.
   *
   *  \return
   *    `true` if there is no contained parser,
   *    `false` otherwise.
   */
  bool done() const noexcept {
    return std::holds_alternative<std::monostate>(parser_);
  }
#ifndef MCPP_DOXYGEN_RUNNING
  bool Null() {
    auto f = [&](auto&& parser) { return parser.Null(); };
    return visit(f);
  }
  bool Bool(bool b) {
    auto f = [&](auto&& parser) { return parser.Bool(b); };
    return visit(f);
  }
  bool Int(int i) {
    auto f = [&](auto&& parser) { return parser.Int(i); };
    return visit(f);
  }
  bool Uint(unsigned u) {
    auto f = [&](auto&& parser) { return parser.Uint(u); };
    return visit(f);
  }
  bool Int64(std::int64_t i) {
    auto f = [&](auto&& parser) { return parser.Int64(i); };
    return visit(f);
  }
  bool Uint64(std::uint64_t u) {
    auto f = [&](auto&& parser) { return parser.Uint64(u); };
    return visit(f);
  }
  bool Double(double d) {
    auto f = [&](auto&& parser) { return parser.Double(d); };
    return visit(f);
  }
  bool String(const char* str,
              std::size_t size,
              bool copy)
  {
    auto f = [&](auto&& parser) { return parser.String(str,
                                                       size,
                                                       copy); };
    return visit(f);
  }
  bool StartObject() {
    auto f = [&](auto&& parser) { return parser.StartObject(); };
    return visit(f);
  }
  bool Key(const char* str,
           std::size_t size,
           bool copy)
  {
    auto f = [&](auto&& parser) { return parser.Key(str,
                                                    size,
                                                    copy); };
    return visit(f);
  }
  bool EndObject(std::size_t size) {
    auto f = [&](auto&& parser) { return parser.EndObject(size); };
    return visit(f);
  }
  bool StartArray() {
    auto f = [&](auto&& parser) { return parser.StartArray(); };
    return visit(f);
  }
  bool EndArray(std::size_t size) {
    auto f = [&](auto&& parser) { return parser.EndArray(size); };
    return visit(f);
  }
  bool RawNumber(const char* str,
                 std::size_t size,
                 bool copy)
  {
    auto f = [&](auto&& parser) { return parser.RawNumber(str,
                                                          size,
                                                          copy); };
    return visit(f);
  }
#endif
protected:
  /**
   *  Constructs a parser in the managed storage.
   *
   *  If there is already a parser in the managed storage
   *  the behavior is undefined.
   *
   *  \tparam T
   *    The type of parser to construct. Must be one of
   *    the types used to instantiate this class template.
   *  \tparam Args
   *    The types of arguments to forward through to a
   *    constructor of `T`.
   *
   *  \param [in] args
   *    Objects to forward through to a construct of
   *    `T`.
   */
  template<typename T,
           typename... Args>
  void emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T,
                                                                        Args&&...>)
  {
    assert(done());
    parser_.template emplace<T>(std::forward<Args>(args)...);
  }
private:
  template<typename Visitor>
  bool visit(Visitor v) {
    bool done = false;
    auto f = [&](auto&& parser) {
      if constexpr (std::is_same_v<std::decay_t<decltype(parser)>,
                                   std::monostate>)
      {
        return v(static_cast<parser_base&>(*this));
      } else {
        auto retr = v(parser);
        if (!retr) {
          error_code(parser.error_code());
          return false;
        }
        if (parser.done()) {
          done = true;
        }
        return true;
      }
    };
    auto retr = std::visit(f,
                           parser_);
    if (!retr || done) {
      clear();
    }
    return retr;
  }
  using variant_type = std::variant<std::monostate,
                                    Parsers...>;
  variant_type parser_;
};

}
