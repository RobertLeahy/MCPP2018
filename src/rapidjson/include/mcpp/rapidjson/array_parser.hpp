/**
 *  \file
 */

#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <vector>
#include "done_parser_base.hpp"

namespace mcpp::rapidjson {

/**
 *  Parses an instantiation of the `std::vector` class
 *  template from a JSON array.
 *
 *  \tparam Element
 *    The type of each element to parse.
 *  \tparam Parser
 *    An object which may be used to parse elements of
 *    type `Element`. Must be constructible with a single
 *    argument of type `Element&` in which case the
 *    constructed object is a parser for the referred to
 *    object.
 *  \tparam Allocator
 *    The `Allocator` on which to instantiation the
 *    `std::vector` class template. Defaults to
 *    `std::allocator<Element>`.
 */
template<typename Element,
         typename Parser,
         typename Allocator = std::allocator<Element>>
class array_parser
#ifndef MCPP_DOXYGEN_RUNNING
: public done_parser_base
#endif
{
public:
  /**
   *  An instantiation of the `std::vector` class template
   *  with the appropriate template arguments given the
   *  arguments used to instantiate this class.
   */
  using vector_type = std::vector<Element,
                                  Allocator>;
  /**
   *  Creates a new array_parser.
   *
   *  \param [in] vec
   *    The `std::vector` into which to parse. This reference
   *    must remain valid until this object receives the last
   *    SAX event.
   */
  explicit array_parser(vector_type& vec)
    : vec_  (&vec),
      begin_(false)
  {
    vec_->clear();
  }
#ifndef MCPP_DOXYGEN_RUNNING
  void clear() noexcept {
    assert(vec_);
    vec_->clear();
    begin_ = false;
    parser_.reset();
    done_parser_base::clear();
  }
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
    if (!begin_) {
      begin_ = true;
      return true;
    }
    auto f = [&](auto&& parser) { return parser.StartArray(); };
    return visit(f);
  }
  bool EndArray(std::size_t size) {
    if (parser_) {
      auto f = [&](auto&& parser) { return parser.EndArray(size); };
      return visit(f);
    }
    finish();
    return true;
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
private:
  template<typename Visitor>
  bool visit(Visitor v) {
    if (!begin_) {
      assert(!parser_);
      return v(static_cast<done_parser_base&>(*this));
    }
    if (!parser_) {
      vec_->emplace_back();
      parser_.emplace(vec_->back());
    }
    assert(parser_);
    if (!v(*parser_)) {
      return false;
    }
    if (parser_->done()) {
      parser_.reset();
    }
    return true;
  }
  vector_type* vec_;
  bool                  begin_;
  std::optional<Parser> parser_;
};

}
